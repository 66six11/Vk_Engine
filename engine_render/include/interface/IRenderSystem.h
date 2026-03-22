#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class IRenderGraphManager;
class IResourceManager;
class IPipelineManager;
class IShaderManager;
class IRenderDebug;
class IRenderProfiler;
class ICommandQueue;
class ISwapChain;
class IRenderDevice;

/**
 * 渲染系统顶层接口
 * 负责初始化渲染子系统并提供对各个管理器的访问
 */
class IRenderSystem {
public:
    virtual ~IRenderSystem() = default;

    // 初始化与销毁
    virtual bool initialize(const RenderSystemConfig& config) = 0;
    virtual void shutdown() = 0;

    // 帧管理
    virtual void beginFrame(const FrameInfo& frameInfo) = 0;
    virtual void endFrame() = 0;
    virtual void present(ISwapChain* swapChain = nullptr) = 0;

    // 子系统访问
    virtual IRenderGraphManager* getRenderGraphManager() = 0;
    virtual IResourceManager* getResourceManager() = 0;
    virtual IPipelineManager* getPipelineManager() = 0;
    virtual IShaderManager* getShaderManager() = 0;
    virtual IRenderDevice* getRenderDevice() = 0;

    // 查询功能
    virtual RenderAPI getRenderAPI() const = 0;
    virtual const DeviceInfo& getDeviceInfo() const = 0;

    // 调试与工具
    virtual IRenderDebug* getDebugInterface() = 0;
    virtual IRenderProfiler* getProfiler() = 0;

    // 多队列支持
    virtual ICommandQueue* getGraphicsQueue(u32 index = 0) = 0;
    virtual ICommandQueue* getComputeQueue(u32 index = 0) = 0;
    virtual ICommandQueue* getTransferQueue(u32 index = 0) = 0;

    // 队列配置
    virtual u32 getGraphicsQueueFamilyIndex() const = 0;
    virtual u32 getComputeQueueFamilyIndex() const = 0;
    virtual u32 getTransferQueueFamilyIndex() const = 0;
    virtual bool isAsyncComputeSupported() const = 0;
    virtual bool isAsyncTransferSupported() const = 0;

    // 队列同步
    virtual void signalQueueFence(QueueType queueType, u64 value) = 0;
    virtual void waitQueueFence(QueueType queueType, u64 value) = 0;
    virtual void signalSemaphore(QueueType queueType, SemaphoreHandle semaphore, u64 value) = 0;

    // 交换链管理
    virtual ISwapChain* createSwapChain(const struct SwapChainCreateInfo& createInfo) = 0;
    virtual void destroySwapChain(ISwapChain* swapChain) = 0;

    // 设备控制
    virtual void waitIdle() = 0;
    virtual void flush() = 0;

    // 统计信息
    virtual MemoryStatistics getMemoryStatistics() const = 0;
};

/**
 * 交换链创建信息
 */
struct SwapChainCreateInfo {
    void* windowHandle = nullptr;
    u32 width = 1280;
    u32 height = 720;
    Format format = Format::RGBA8_UNORM;
    u32 bufferCount = 3;
    bool vsync = true;
    bool fullscreen = false;
    bool hdr = false;
};

/**
 * 交换链接口
 */
class ISwapChain {
public:
    virtual ~ISwapChain() = default;

    // 获取当前后备缓冲区
    virtual TextureHandle getCurrentBackBuffer() = 0;
    virtual ResourceHandle getCurrentBackBufferResource() = 0;

    // 呈现
    virtual void present(bool vsync = true) = 0;

    // 调整大小
    virtual void resize(u32 width, u32 height) = 0;

    // 查询
    virtual u32 getWidth() const = 0;
    virtual u32 getHeight() const = 0;
    virtual Format getFormat() const = 0;
    virtual u32 getBufferCount() const = 0;
    virtual u32 getCurrentBufferIndex() const = 0;

    // 等待
    virtual void waitForPresent() = 0;
};

/**
 * @brief 调试消息严重级别
 * @details 验证层消息的优先级分类
 */
enum class DebugSeverity : u32 {
    Verbose = 0,    ///< 详细/调试信息
    Info = 1,       ///< 普通信息
    Warning = 2,    ///< 警告
    Error = 3,      ///< 错误
    Fatal = 4       ///< 致命错误
};

/**
 * @brief 调试消息类型
 * @details 验证层消息的分类
 */
enum class DebugMessageType : u32 {
    General = 1 << 0,       ///< 一般消息
    Validation = 1 << 1,    ///< 验证错误
    Performance = 1 << 2,   ///< 性能警告
    Portability = 1 << 3,   ///< 可移植性问题
    All = 0xFFFFFFFF        ///< 所有类型
};

inline DebugMessageType operator|(DebugMessageType a, DebugMessageType b) {
    return static_cast<DebugMessageType>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

/**
 * 调试回调函数类型
 */
using DebugCallbackFn = std::function<void(
    DebugSeverity severity,
    DebugMessageType type,
    const char* message,
    const char* objectName,
    void* userData)>;

/**
 * 渲染调试接口
 * 提供GPU调试标记、对象命名、消息回调等功能
 */
class IRenderDebug {
public:
    virtual ~IRenderDebug() = default;

    // ==================== 调试区域标记 ====================
    virtual void beginDebugRegion(const char* name, const Color& color) = 0;
    virtual void endDebugRegion() = 0;
    virtual void insertDebugMarker(const char* name, const Color& color) = 0;

    // ==================== 对象命名 ====================
    virtual void setObjectName(ResourceHandle handle, const char* name) = 0;
    virtual void setObjectName(TextureHandle handle, const char* name) = 0;
    virtual void setObjectName(BufferHandle handle, const char* name) = 0;
    virtual void setObjectName(PipelineHandle handle, const char* name) = 0;
    virtual void setObjectName(SamplerHandle handle, const char* name) = 0;
    virtual void setObjectName(DescriptorSetHandle handle, const char* name) = 0;
    virtual void setObjectName(CommandBufferHandle handle, const char* name) = 0;
    virtual void setObjectName(RenderPassHandle handle, const char* name) = 0;
    virtual void setObjectName(FramebufferHandle handle, const char* name) = 0;

    // ==================== 调试输出配置 ====================
    virtual void enableDebugOutput(bool enable) = 0;
    virtual void setDebugBreakSeverity(DebugSeverity severity) = 0;
    virtual void setDebugMessageFilter(DebugMessageType types) = 0;
    virtual void setDebugCallback(DebugCallbackFn callback, void* userData = nullptr) = 0;

    // ==================== 资源状态可视化 ====================
    virtual void dumpResourceState(const char* filename) = 0;
    virtual void dumpTextureState(TextureHandle handle, const char* filename = nullptr) = 0;
    virtual void dumpBufferState(BufferHandle handle, const char* filename = nullptr) = 0;
    virtual void visualizeResourceDependencies(const char* outputPath) = 0;

    // ==================== 命令缓冲区调试 ====================
    virtual void dumpCommandBuffer(const char* filename) = 0;
    virtual void validateCommandBuffer() = 0;

    // ==================== 查询功能 ====================
    [[nodiscard]] virtual bool isDebugOutputEnabled() const = 0;
    [[nodiscard]] virtual DebugSeverity getDebugBreakSeverity() const = 0;
    virtual const char* getObjectName(ResourceHandle handle) const = 0;
};

/**
 * @brief 性能分析器采样模式
 * @details GPU性能分析的采样频率
 */
enum class ProfilerSamplingMode {
    Disabled,       ///< 完全禁用
    Frame,          ///< 每帧采样
    Interval,       ///< 按间隔采样
    Continuous      ///< 连续采样
};

/**
 * 性能分析统计项
 */
struct ProfileStats {
    u32 drawCalls = 0;
    u32 triangles = 0;
    u32 dispatches = 0;
    u32 barriers = 0;
    u32 pipelineSwitches = 0;
    u32 descriptorSetSwitches = 0;
    u32 renderTargetSwitches = 0;
    u64 vertices = 0;
    u64 primitives = 0;
    u32 computeInvocations = 0;
};

/**
 * GPU时间戳数据
 */
struct GPUTimestamp {
    const char* name = nullptr;
    double startTimeMs = 0.0;
    double endTimeMs = 0.0;
    double durationMs = 0.0;
    u32 depth = 0;  // 嵌套深度
};

/**
 * 帧性能数据
 */
struct FrameProfileData {
    u64 frameNumber = 0;
    double cpuFrameTimeMs = 0.0;
    double gpuFrameTimeMs = 0.0;
    ProfileStats stats;
    std::vector<GPUTimestamp> timestamps;
    MemoryStatistics memoryStats;
};

/**
 * 渲染性能分析器接口
 * 提供GPU/CPU性能分析、时间线、自动采样等功能
 */
class IRenderProfiler {
public:
    virtual ~IRenderProfiler() = default;

    // ==================== 采样控制 ====================
    virtual void setSamplingMode(ProfilerSamplingMode mode) = 0;
    virtual void setSamplingInterval(u32 intervalFrames) = 0;  // 间隔采样模式下的帧间隔
    virtual void enableAutoSampling(bool enable) = 0;

    // ==================== GPU时间戳 ====================
    virtual void beginGpuTimestamp(const char* name) = 0;
    virtual void endGpuTimestamp(const char* name) = 0;
    [[nodiscard]] virtual double getGpuTimestamp(const char* name) const = 0;
    [[nodiscard]] virtual std::vector<GPUTimestamp> getAllGpuTimestamps() const = 0;

    // ==================== CPU时间戳 ====================
    virtual void beginCpuTimestamp(const char* name) = 0;
    virtual void endCpuTimestamp(const char* name) = 0;
    [[nodiscard]] virtual double getCpuTimestamp(const char* name) const = 0;

    // ==================== 帧管理 ====================
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    [[nodiscard]] virtual u64 getCurrentFrameNumber() const = 0;

    // ==================== 统计更新（由渲染器调用）====================
    virtual void incrementDrawCalls(u32 count = 1) = 0;
    virtual void incrementTriangles(u32 count) = 0;
    virtual void incrementDispatches(u32 count = 1) = 0;
    virtual void incrementBarriers(u32 count = 1) = 0;
    virtual void incrementPipelineSwitches(u32 count = 1) = 0;
    virtual void incrementDescriptorSetSwitches(u32 count = 1) = 0;
    virtual void incrementRenderTargetSwitches(u32 count = 1) = 0;
    virtual void incrementVertices(u64 count) = 0;
    virtual void incrementPrimitives(u64 count) = 0;
    virtual void incrementComputeInvocations(u32 count) = 0;

    // ==================== 查询统计 ====================
    [[nodiscard]] virtual u32 getDrawCallCount() const = 0;
    [[nodiscard]] virtual u32 getTriangleCount() const = 0;
    [[nodiscard]] virtual u32 getDispatchCount() const = 0;
    [[nodiscard]] virtual u32 getBarrierCount() const = 0;
    [[nodiscard]] virtual u32 getPipelineSwitchCount() const = 0;
    [[nodiscard]] virtual u32 getDescriptorSetSwitchCount() const = 0;
    [[nodiscard]] virtual u32 getRenderTargetSwitchCount() const = 0;
    [[nodiscard]] virtual const ProfileStats& getCurrentStats() const = 0;

    // ==================== 帧数据查询 ====================
    [[nodiscard]] virtual FrameProfileData getCurrentFrameData() const = 0;
    [[nodiscard]] virtual std::vector<FrameProfileData> getFrameHistory(u32 maxFrames = 60) const = 0;
    [[nodiscard]] virtual double getAverageFrameTimeMs(u32 frameCount = 60) const = 0;
    [[nodiscard]] virtual double getMinFrameTimeMs(u32 frameCount = 60) const = 0;
    [[nodiscard]] virtual double getMaxFrameTimeMs(u32 frameCount = 60) const = 0;

    // ==================== 报告输出 ====================
    virtual void dumpProfileData(const char* filename) = 0;
    virtual void dumpFrameData(const char* filename, u32 frameCount = 1) = 0;
    virtual void exportToChromeTracing(const char* filename) = 0;  // Chrome tracing格式
    virtual void reset() = 0;
    virtual void resetStats() = 0;

    // ==================== 实时查询 ====================
    [[nodiscard]] virtual bool isProfilingEnabled() const = 0;
    [[nodiscard]] virtual ProfilerSamplingMode getSamplingMode() const = 0;
};

/**
 * 渲染系统工厂
 */
class RenderSystemFactory {
public:
    static std::unique_ptr<IRenderSystem> create(RenderAPI api);
    static std::unique_ptr<IRenderSystem> createDefault();
    static std::vector<RenderAPI> getSupportedAPIs();
};

} // namespace render