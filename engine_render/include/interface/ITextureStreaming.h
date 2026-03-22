#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include "utils/Math.h"
#include <functional>

namespace render {

// 数学类型
using math::Vector2;
using math::Vector3;

// 前向声明
class IResourceManager;
class IRenderJobSystem;
class ICamera;

/**
 * @brief 纹理流送优先级
 * @details 纹理数据异步加载的优先级
 */
enum class StreamPriority : u8 {
    Low = 0,        ///< 低优先级（后台流送）
    Normal = 1,     ///< 正常优先级
    High = 2,       ///< 高优先级（尽快加载）
    Critical = 3    ///< 关键优先级（阻塞式立即加载）
};

/**
 * @brief 纹理流送状态
 * @details 纹理资源在流送系统中的当前状态
 */
enum class StreamStatus : u8 {
    Unloaded,       ///< 未加载（仅元数据）
    Pending,        ///< 等待加载队列
    Loading,        ///< 加载中
    Resident,       ///< 已驻留（数据在GPU内存中可用）
    Evicting        ///< 正在卸载（为更高优先级纹理腾出空间）
};

/**
 * Mip加载状态
 */
struct MipLoadState {
    u32 mipLevel;
    StreamStatus status;
    u64 dataSize;
    float loadProgress;  // 0.0 - 1.0
};

/**
 * 纹理流送设置
 */
struct TextureStreamingSettings {
    // 基础设置
    u32 maxResidentMips = 0;          // 0 = 自动（根据距离计算）
    u32 minResidentMips = 1;          // 至少保留的MIP层级
    StreamPriority priority = StreamPriority::Normal;
    
    // 距离LOD
    float lodDistance0 = 10.0f;       // 最高质量距离
    float lodDistance1 = 50.0f;       // 中等质量距离
    float lodDistance2 = 200.0f;      // 低质量距离
    
    // 流送控制
    bool allowEviction = true;        // 允许卸载
    bool preloadNearby = true;        // 预加载附近纹理
    float preloadDistance = 1.5f;     // 预加载距离倍数
    
    // 动画纹理
    bool isAnimated = false;          // 是否动态更新
    u32 animationFps = 30;            // 动画帧率
    
    // 内存预算
    u64 maxMemoryBudget = 0;          // 0 = 使用全局预算
};

/**
 * 流送统计
 */
struct StreamingStats {
    // 纹理统计
    u32 totalTextures = 0;
    u32 streamingTextures = 0;
    u32 residentTextures = 0;
    u32 loadingTextures = 0;
    
    // 内存统计
    u64 totalTextureMemory = 0;
    u64 residentMemory = 0;
    u64 pendingMemory = 0;
    u64 budget = 0;
    float utilization = 0.0f;
    
    // 流送统计
    u32 loadsThisFrame = 0;
    u32 evictionsThisFrame = 0;
    u64 bytesStreamedThisFrame = 0;
    double streamTimeThisFrame = 0.0;
    
    // 缓存命中
    u32 cacheHits = 0;
    u32 cacheMisses = 0;
    float cacheHitRate = 0.0f;
    
    // 队列
    u32 pendingRequests = 0;
    u32 activeRequests = 0;
};

/**
 * 纹理流送信息
 */
struct TextureStreamInfo {
    TextureHandle handle;
    const char* name = nullptr;
    StreamStatus status = StreamStatus::Unloaded;
    u32 currentMipCount = 0;
    u32 maxMipCount = 0;
    u32 targetMipCount = 0;
    u64 residentSize = 0;
    u64 fullSize = 0;
    float priorityScore = 0.0f;
    float distanceToCamera = 0.0f;
    Vector2 screenSize;  // 屏幕空间大小
};

/**
 * 流送请求
 */
struct StreamRequest {
    TextureHandle texture;
    u32 targetMipLevel;       // 目标MIP层级（0 = 最高质量）
    StreamPriority priority;
    float urgency = 1.0f;     // 紧急程度（1.0 = 正常）
    std::function<void(bool success)> onComplete;
};

/**
 * 磁盘缓存配置
 */
struct TextureDiskCacheConfig {
    bool enableCache = true;
    const char* cacheDirectory = "cache/textures/";
    u64 maxCacheSize = 1024ull * 1024 * 1024;  // 1GB
    bool compressCache = true;
    u32 compressionQuality = 85;  // 0-100
};

/**
 * 纹理流送配置
 */
struct TextureStreamingConfig {
    // 内存预算
    u64 memoryBudget = 512ull * 1024 * 1024;  // 512MB默认预算
    float criticalThreshold = 0.95f;            // 95%触发紧急卸载
    float warningThreshold = 0.85f;             // 85%警告
    
    // 流送限制
    u32 maxConcurrentLoads = 4;               // 最大并发加载数
    u32 maxMipsPerFrame = 2;                  // 每帧最大MIP更新数
    u64 maxBytesPerFrame = 64ull * 1024 * 1024;  // 每帧最大64MB
    
    // 流送策略
    bool useVisibilityInfo = true;            // 使用可见性信息
    bool useDistanceBasedLOD = true;          // 使用距离LOD
    bool useScreenSizeLOD = true;             // 使用屏幕大小LOD
    bool enableAsyncUpload = true;            // 异步上传
    
    // 预加载
    bool enableLookAhead = true;              // 预加载
    u32 lookAheadFrames = 2;                  // 提前2帧
    float lookAheadDistance = 1.2f;           // 预加载距离倍数
    
    // 磁盘缓存
    TextureDiskCacheConfig diskCache;
    
    // 调试
    bool logStreamingEvents = false;
    bool visualizeMipLevels = false;
};

/**
 * 纹理流送接口
 * 
 * 提供纹理的动态流送、LOD管理、内存预算控制等功能
 * 
 * 使用示例：
 * @code
 * // 1. 创建流送系统
 * auto streaming = render::TextureStreamingFactory::create(config);
 * streaming->initialize(resourceManager, jobSystem);
 * 
 * // 2. 注册纹理（启用流送）
 * TextureHandle texture = resourceMgr->createTexture({...});
 * streaming->registerTexture(texture, {
 *     .priority = StreamPriority::Normal,
 *     .lodDistance0 = 10.0f,
 *     .lodDistance1 = 50.0f
 * });
 * 
 * // 3. 每帧更新（根据相机位置调整）
 * streaming->update(camera, deltaTime);
 * 
 * // 4. 手动请求特定MIP（例如近距离观察）
 * streaming->requestMipLevel(texture, 0, StreamPriority::High);
 * 
 * // 5. 查询状态
 * auto info = streaming->getTextureInfo(texture);
 * if (info.status == StreamStatus::Resident) {
 *     // 纹理已就绪
 * }
 * @endcode
 */
class ITextureStreaming {
public:
    virtual ~ITextureStreaming() = default;

    // ==================== 初始化与配置 ====================
    
    /**
     * @brief 初始化流送系统
     */
    virtual bool initialize(IResourceManager* resourceManager,
                           IRenderJobSystem* jobSystem,
                           const TextureStreamingConfig& config) = 0;
    
    /**
     * @brief 关闭流送系统
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 更新配置
     */
    virtual void setConfig(const TextureStreamingConfig& config) = 0;
    
    /**
     * @brief 获取当前配置
     */
    [[nodiscard]] virtual const TextureStreamingConfig& getConfig() const = 0;
    
    /**
     * @brief 设置内存预算
     */
    virtual void setMemoryBudget(u64 budget) = 0;
    
    /**
     * @brief 获取内存预算
     */
    [[nodiscard]] virtual u64 getMemoryBudget() const = 0;

    // ==================== 纹理注册 ====================
    
    /**
     * @brief 注册纹理到流送系统
     * @param texture 纹理句柄
     * @param settings 流送设置
     * @return 是否成功
     */
    virtual bool registerTexture(TextureHandle texture,
                                const TextureStreamingSettings& settings) = 0;
    
    /**
     * @brief 注销纹理
     */
    virtual void unregisterTexture(TextureHandle texture) = 0;
    
    /**
     * @brief 更新纹理流送设置
     */
    virtual void updateTextureSettings(TextureHandle texture,
                                      const TextureStreamingSettings& settings) = 0;
    
    /**
     * @brief 检查纹理是否已注册
     */
    [[nodiscard]] virtual bool isTextureRegistered(TextureHandle texture) const = 0;
    
    /**
     * @brief 获取注册的所有纹理
     */
    [[nodiscard]] virtual std::vector<TextureHandle> getRegisteredTextures() const = 0;

    // ==================== MIP管理（核心功能）====================
    
    /**
     * @brief 请求特定MIP层级
     * @param texture 纹理句柄
     * @param mipLevel 目标MIP层级（0 = 最高质量）
     * @param priority 优先级
     * @return 请求ID（用于查询状态）
     * @note 系统会尽快加载到该MIP或更高
     */
    [[nodiscard]] virtual u32 requestMipLevel(TextureHandle texture,
                                             u32 mipLevel,
                                             StreamPriority priority = StreamPriority::Normal) = 0;
    
    /**
     * @brief 请求完整纹理（最高质量）
     * @param texture 纹理句柄
     * @param priority 优先级
     * @param callback 完成回调
     * @return 请求ID
     */
    [[nodiscard]] virtual u32 requestFullRes(TextureHandle texture,
                                            StreamPriority priority = StreamPriority::Normal,
                                            std::function<void(bool)> callback = nullptr) = 0;
    
    /**
     * @brief 预加载纹理（后台低优先级）
     */
    virtual void prefetchTexture(TextureHandle texture,
                                u32 targetMipLevel = 0) = 0;
    
    /**
     * @brief 强制卸载纹理（释放内存）
     */
    virtual void evictTexture(TextureHandle texture,
                             u32 targetMipLevel = 1) = 0;
    
    /**
     * @brief 等待纹理就绪
     * @param texture 纹理句柄
     * @param targetMipLevel 目标MIP
     * @param timeoutMs 超时（毫秒）
     * @return 是否就绪
     */
    virtual bool waitForTexture(TextureHandle texture,
                               u32 targetMipLevel = 0,
                               u64 timeoutMs = 0) = 0;
    
    /**
     * @brief 获取当前MIP数量
     */
    [[nodiscard]] virtual u32 getCurrentMipCount(TextureHandle texture) const = 0;
    
    /**
     * @brief 获取MIP加载状态
     */
    [[nodiscard]] virtual std::vector<MipLoadState> getMipLoadStates(TextureHandle texture) const = 0;

    // ==================== 每帧更新（核心）====================
    
    /**
     * @brief 更新流送系统（每帧调用）
     * @param camera 当前相机
     * @param deltaTime 时间增量
     * @note 根据距离和可见性自动调整MIP
     */
    virtual void update(const ICamera& camera, float deltaTime) = 0;
    
    /**
     * @brief 批量更新纹理可见性
     * @param visibleTextures 可见纹理列表
     * @param cameraPosition 相机位置
     * @note 通常由场景管理器调用
     */
    virtual void updateVisibility(const std::vector<TextureHandle>& visibleTextures,
                                  const Vector3& cameraPosition) = 0;
    
    /**
     * @brief 设置纹理世界位置（用于距离计算）
     */
    virtual void setTextureWorldPosition(TextureHandle texture, const Vector3& position) = 0;
    
    /**
     * @brief 设置纹理屏幕大小（用于屏幕LOD）
     */
    virtual void setTextureScreenSize(TextureHandle texture, const Vector2& screenSize) = 0;

    // ==================== 异步加载 ====================
    
    /**
     * @brief 处理流送请求
     * @param maxRequests 最大处理请求数
     * @return 处理的请求数
     * @note 通常在后台线程调用
     */
    virtual u32 processRequests(u32 maxRequests = 4) = 0;
    
    /**
     * @brief 提交流送请求
     */
    virtual void submitRequest(const StreamRequest& request) = 0;
    
    /**
     * @brief 取消流送请求
     */
    virtual void cancelRequest(u32 requestId) = 0;
    
    /**
     * @brief 检查请求是否完成
     */
    [[nodiscard]] virtual bool isRequestCompleted(u32 requestId) const = 0;

    // ==================== 内存管理 ====================
    
    /**
     * @brief 获取当前内存使用
     */
    [[nodiscard]] virtual u64 getCurrentMemoryUsage() const = 0;
    
    /**
     * @brief 获取指定纹理的内存使用
     */
    [[nodiscard]] virtual u64 getTextureMemoryUsage(TextureHandle texture) const = 0;
    
    /**
     * @brief 强制内存回收
     * @param targetMemory 目标内存使用量
     * @return 释放的内存量
     */
    virtual u64 reclaimMemory(u64 targetMemory) = 0;
    
    /**
     * @brief 紧急卸载（内存不足时调用）
     */
    virtual void emergencyEviction() = 0;
    
    /**
     * @brief 检查是否需要回收内存
     */
    [[nodiscard]] virtual bool needsReclaim() const = 0;

    // ==================== 磁盘缓存 ====================
    
    /**
     * @brief 预热磁盘缓存
     * @param texturePaths 纹理路径列表
     * @note 将纹理预处理后存入磁盘缓存
     */
    virtual void warmupDiskCache(const std::vector<const char*>& texturePaths) = 0;
    
    /**
     * @brief 清理磁盘缓存
     * @param maxAgeDays 最大保留天数
     * @return 释放的空间
     */
    virtual u64 purgeDiskCache(u32 maxAgeDays = 30) = 0;
    
    /**
     * @brief 获取磁盘缓存大小
     */
    [[nodiscard]] virtual u64 getDiskCacheSize() const = 0;

    // ==================== 查询与统计 ====================
    
    /**
     * @brief 获取纹理流送信息
     */
    [[nodiscard]] virtual TextureStreamInfo getTextureInfo(TextureHandle texture) const = 0;
    
    /**
     * @brief 获取所有流送信息
     */
    [[nodiscard]] virtual std::vector<TextureStreamInfo> getAllTextureInfo() const = 0;
    
    /**
     * @brief 获取流送统计
     */
    [[nodiscard]] virtual StreamingStats getStats() const = 0;
    
    /**
     * @brief 重置统计
     */
    virtual void resetStats() = 0;
    
    /**
     * @brief 获取指定状态的纹理
     */
    [[nodiscard]] virtual std::vector<TextureHandle> getTexturesByStatus(
        StreamStatus status) const = 0;
    
    /**
     * @brief 导出流送报告
     */
    virtual void exportReport(const char* filename) const = 0;

    // ==================== 调试与可视化 ====================
    
    /**
     * @brief 可视化MIP边界（调试用）
     * @param cmdBuffer 命令缓冲区
     * @param texture 纹理
     */
    virtual void visualizeMipLevels(ICommandBuffer* cmdBuffer, TextureHandle texture) = 0;
    
    /**
     * @brief 打印流送统计
     */
    virtual void printStats() const = 0;
    
    /**
     * @brief 获取流送时间线
     */
    virtual void exportStreamingTimeline(const char* filename) const = 0;
};

/**
 * @brief 纹理流送工厂
 */
class TextureStreamingFactory {
public:
    static std::unique_ptr<ITextureStreaming> create();
    static std::unique_ptr<ITextureStreaming> create(const TextureStreamingConfig& config);
};

// ==================== 便捷宏 ====================

/**
 * @brief 注册纹理流送
 */
#define REGISTER_STREAMING_TEXTURE(streaming, texture, settings) \
    (streaming)->registerTexture((texture), (settings))

/**
 * @brief 请求最高质量
 */
#define REQUEST_FULL_RES(streaming, texture) \
    (streaming)->requestFullRes((texture), render::StreamPriority::High)

/**
 * @brief 等待纹理就绪（带超时）
 */
#define WAIT_TEXTURE_READY(streaming, texture, timeout) \
    (streaming)->waitForTexture((texture), 0, (timeout))

/**
 * @brief 作用域纹理预加载
 */
class TexturePrefetchScope {
public:
    TexturePrefetchScope(ITextureStreaming* system, TextureHandle tex, u32 mip = 0)
        : streaming(system), texture(tex), targetMip(mip) {
        if (streaming) {
            streaming->prefetchTexture(texture, targetMip);
        }
    }
    
    ~TexturePrefetchScope() {
        // 可选：作用域结束时卸载
    }
    
    TexturePrefetchScope(const TexturePrefetchScope&) = delete;
    TexturePrefetchScope& operator=(const TexturePrefetchScope&) = delete;
    TexturePrefetchScope(TexturePrefetchScope&&) = delete;
    TexturePrefetchScope& operator=(TexturePrefetchScope&&) = delete;
    
private:
    ITextureStreaming* streaming;
    TextureHandle texture;
    u32 targetMip;
};

#define TEXTURE_PREFETCH_SCOPE(streaming, texture, mip) \
    render::TexturePrefetchScope _texPrefetch_##__LINE__(streaming, texture, mip)

} // namespace render
