#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include <chrono>

namespace render {

// 前向声明
class IResourceManager;

/**
 * @brief GPU资源类型分类
 * @details 用于资源调试和内存分析的资源分类
 */
enum class GPUResourceCategory {
    Texture,        ///< 纹理资源
    Buffer,         ///< 通用缓冲区
    RenderTarget,   ///< 渲染目标
    DepthStencil,   ///< 深度模板缓冲区
    ShaderResource, ///< 着色器资源
    UploadBuffer,   ///< 上传缓冲区
    ReadbackBuffer, ///< 回读缓冲区
    Count           ///< 分类数量
};

/**
 * 资源分配信息
 */
struct ResourceAllocationInfo {
    ResourceHandle handle;
    const char* name = nullptr;
    GPUResourceCategory category = GPUResourceCategory::Texture;
    u64 size = 0;
    u64 alignment = 0;
    MemoryType memoryType = MemoryType::Default;
    u32 bindCount = 0;
    u32 frameCreated = 0;
    u32 lastUsedFrame = 0;
    bool isTransient = false;
    bool isAliased = false;
    ResourceHandle aliasedWith;
};

/**
 * 资源使用追踪信息
 */
struct ResourceUsageInfo {
    ResourceHandle handle;
    u32 bindFrame = 0;
    u32 bindPass = 0;
    ResourceState state = ResourceState::Undefined;
    std::chrono::steady_clock::time_point timestamp;
};

/**
 * 内存堆信息
 */
struct MemoryHeapInfo {
    u64 totalSize = 0;
    u64 usedSize = 0;
    u64 freeSize = 0;
    u64 largestFreeBlock = 0;
    u32 allocationCount = 0;
    MemoryType type = MemoryType::Default;
    float fragmentation = 0.0f;  // 0-1，碎片率
};

/**
 * 资源泄漏检测信息
 */
struct ResourceLeakInfo {
    ResourceHandle handle;
    const char* name = nullptr;
    GPUResourceCategory category = GPUResourceCategory::Texture;
    u64 size = 0;
    u32 frameCreated = 0;
    u32 framesUnused = 0;
    u32 bindCount = 0;
    const char* allocationStack = nullptr;  // 分配堆栈（如果支持）
};

/**
 * 资源性能统计
 */
struct ResourcePerformanceStats {
    u32 totalAllocations = 0;
    u32 activeAllocations = 0;
    u32 transientAllocations = 0;
    u32 aliasedAllocations = 0;
    u64 totalMemory = 0;
    u64 deviceMemory = 0;
    u64 uploadMemory = 0;
    u64 readbackMemory = 0;
    u64 transientMemory = 0;
    u32 bindCountThisFrame = 0;
    u32 stateTransitionCount = 0;
    double averageAllocationTimeMs = 0.0;
    double peakAllocationTimeMs = 0.0;
};

/**
 * @brief 内存预算警告级别
 * @details GPU内存使用量的警告等级
 */
enum class MemoryWarningLevel {
    None,       ///< 正常（< 50%）
    Info,       ///< 信息（> 50%）
    Warning,    ///< 警告（> 75%）
    Critical    ///< 危险（> 90%）
};

/**
 * GPU资源调试器
 * 提供资源追踪、内存分析、泄漏检测等功能
 */
class GPUResourceDebugger {
public:
    explicit GPUResourceDebugger(IResourceManager* resourceManager);
    ~GPUResourceDebugger();

    // 禁用拷贝
    GPUResourceDebugger(const GPUResourceDebugger&) = delete;
    GPUResourceDebugger& operator=(const GPUResourceDebugger&) = delete;

    // ==================== 资源追踪 ====================
    void trackResourceCreation(ResourceHandle handle, GPUResourceCategory category,
                               u64 size, const char* name = nullptr);
    void trackResourceDestruction(ResourceHandle handle);
    void trackResourceBind(ResourceHandle handle, ResourceState state);
    void trackResourceTransition(ResourceHandle handle, ResourceState oldState, ResourceState newState);
    void trackMemoryAllocation(void* address, u64 size, MemoryType type);
    void trackMemoryFree(void* address);

    // ==================== 资源查询 ====================
    [[nodiscard]] ResourceAllocationInfo getResourceInfo(ResourceHandle handle) const;
    [[nodiscard]] std::vector<ResourceAllocationInfo> getAllResources() const;
    [[nodiscard]] std::vector<ResourceAllocationInfo> getResourcesByCategory(GPUResourceCategory category) const;
    [[nodiscard]] std::vector<ResourceAllocationInfo> getResourcesByMemoryType(MemoryType type) const;
    [[nodiscard]] std::vector<ResourceAllocationInfo> getUnusedResources(u32 minFramesUnused = 10) const;

    // ==================== 使用历史 ====================
    [[nodiscard]] std::vector<ResourceUsageInfo> getResourceUsageHistory(ResourceHandle handle,
                                                                         u32 maxEntries = 100) const;
    void clearUsageHistory();

    // ==================== 内存分析 ====================
    [[nodiscard]] MemoryHeapInfo getMemoryHeapInfo(MemoryType type) const;
    [[nodiscard]] std::vector<MemoryHeapInfo> getAllMemoryHeaps() const;
    [[nodiscard]] u64 getTotalMemoryUsage() const;
    [[nodiscard]] u64 getMemoryUsageByCategory(GPUResourceCategory category) const;
    [[nodiscard]] float getMemoryFragmentation(MemoryType type) const;
    void dumpMemoryMap(const char* filename) const;
    void visualizeMemoryLayout(const char* filename) const;

    // ==================== 泄漏检测 ====================
    void enableLeakDetection(bool enable);
    void setLeakDetectionThreshold(u32 minFramesUnused);
    [[nodiscard]] std::vector<ResourceLeakInfo> detectLeaks() const;
    void dumpLeakReport(const char* filename) const;

    // ==================== 预算监控 ====================
    void setMemoryBudget(u64 budget);
    void setMemoryWarningThresholds(float info, float warning, float critical);
    [[nodiscard]] MemoryWarningLevel checkMemoryBudget() const;
    [[nodiscard]] float getMemoryUtilization() const;

    // ==================== 统计分析 ====================
    [[nodiscard]] ResourcePerformanceStats getPerformanceStats() const;
    void resetPerformanceStats();
    void dumpStatistics(const char* filename) const;

    // ==================== 快照功能 ====================
    struct ResourceSnapshot {
        u64 timestamp;
        u64 totalMemory;
        u32 allocationCount;
        std::vector<ResourceAllocationInfo> resources;
    };

    void takeSnapshot(const char* name);
    [[nodiscard]] ResourceSnapshot getSnapshot(const char* name) const;
    void compareSnapshots(const char* snapshotA, const char* snapshotB,
                         std::vector<ResourceAllocationInfo>* added,
                         std::vector<ResourceAllocationInfo>* removed,
                         std::vector<std::pair<ResourceAllocationInfo, ResourceAllocationInfo>>* modified) const;
    void clearSnapshots();

    // ==================== 纹理分析 ====================
    struct TextureAnalysis {
        u32 totalTextures = 0;
        u64 totalTextureMemory = 0;
        u32 renderTargetCount = 0;
        u32 shaderResourceCount = 0;
        u32 depthStencilCount = 0;
        std::map<Format, u32> formatDistribution;
        std::map<u32, u32> sizeDistribution;  // 尺寸分布
    };

    [[nodiscard]] TextureAnalysis analyzeTextures() const;
    void findDuplicateTextures(std::vector<std::pair<ResourceHandle, ResourceHandle>>* duplicates) const;
    void findOversizedTextures(u32 maxRecommendedSize,
                               std::vector<ResourceAllocationInfo>* oversized) const;

    // ==================== 缓冲区分析 ====================
    struct BufferAnalysis {
        u32 totalBuffers = 0;
        u64 totalBufferMemory = 0;
        u32 vertexBufferCount = 0;
        u32 indexBufferCount = 0;
        u32 uniformBufferCount = 0;
        u32 storageBufferCount = 0;
        u64 wastedMemory = 0;  // 未使用的分配空间
    };

    [[nodiscard]] BufferAnalysis analyzeBuffers() const;
    void findUninitializedBuffers(std::vector<ResourceHandle>* buffers) const;

    // ==================== 实时可视化 ====================
    void enableRealtimeVisualization(bool enable);
    void updateVisualization();
    [[nodiscard]] bool isRealtimeVisualizationEnabled() const;

    // ==================== 导出报告 ====================
    void exportToJSON(const char* filename) const;
    void exportToCSV(const char* filename) const;
    void exportToHTML(const char* filename) const;

    // ==================== 配置 ====================
    void setTrackStackTraces(bool enable);
    void setMaxHistoryEntries(u32 maxEntries);
    void setFrameNumber(u32 frameNumber);

private:
    IResourceManager* resourceManager;
    
    struct Impl;
    std::unique_ptr<Impl> impl;

    // 辅助函数
    void updateMemoryStats();
    const char* getCategoryName(GPUResourceCategory category) const;
    const char* getMemoryTypeName(MemoryType type) const;
};

/**
 * 便捷的资源调试宏
 */
#define GPU_DEBUG_TRACK_CREATE(handle, category, size) \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        debugger->trackResourceCreation(handle, category, size, #handle); \
    }

#define GPU_DEBUG_TRACK_DESTROY(handle) \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        debugger->trackResourceDestruction(handle); \
    }

#define GPU_DEBUG_TRACK_BIND(handle, state) \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        debugger->trackResourceBind(handle, state); \
    }

#define GPU_DEBUG_DUMP_MEMORY(filename) \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        debugger->dumpMemoryMap(filename); \
    }

#define GPU_DEBUG_CHECK_LEAKS() \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        auto leaks = debugger->detectLeaks(); \
        if (!leaks.empty()) { \
            LOG_WARNING("Detected {} potential resource leaks", leaks.size()); \
            for (const auto& leak : leaks) { \
                LOG_WARNING("  Leak: {} ({} bytes, {} frames unused)", \
                    leak.name ? leak.name : "unnamed", leak.size, leak.framesUnused); \
            } \
        } \
    }

#define GPU_DEBUG_SNAPSHOT(name) \
    if (auto* debugger = render::GPUResourceDebugger::get()) { \
        debugger->takeSnapshot(name); \
        LOG_DEBUG("Resource snapshot '{}' taken", name); \
    }

} // namespace render
