#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include "IPipelineManager.h"

namespace render {

// 前向声明
class IPipelineManager;

/**
 * @brief PSO构建优先级
 * @details 管线状态对象的异步编译优先级
 */
enum class PSOPriority : u8 {
    Low = 0,        ///< 低优先级（后台编译）
    Normal = 1,     ///< 正常优先级
    High = 2,       ///< 高优先级（尽快编译）
    Critical = 3    ///< 关键优先级（阻塞式立即编译）
};

/**
 * @brief PSO编译状态
 * @details 管线状态对象的当前编译状态
 */
enum class PSOCompileStatus : u8 {
    Pending,        ///< 等待编译
    Compiling,      ///< 编译中
    Completed,      ///< 编译完成
    Failed,         ///< 编译失败
    Cached          ///< 从缓存加载
};

/**
 * @brief PSO缓存策略
 * @details 管线状态对象的缓存持久化策略
 */
enum class PSOCacheStrategy : u8 {
    None,           ///< 不缓存
    Memory,         ///< 仅内存缓存
    Disk,           ///< 磁盘持久化缓存
    MemoryAndDisk   ///< 内存+磁盘双重缓存
};

/**
 * PSO预热统计
 */
struct PSOWarmupStats {
    u32 totalPSOs = 0;
    u32 compiledPSOs = 0;
    u32 cachedPSOs = 0;
    u32 failedPSOs = 0;
    double totalTimeMs = 0.0;
    double averageTimeMs = 0.0;
};

/**
 * PSO编译任务描述
 */
struct PSOCompileJob {
    std::variant<GraphicsPipelineDesc, ComputePipelineDesc> desc;
    PSOPriority priority = PSOPriority::Normal;
    const char* debugName = nullptr;
    std::function<void(PipelineHandle, PSOCompileStatus)> onComplete;
};

/**
 * PSO运行时统计
 */
struct PSORuntimeStats {
    u32 totalPSOs = 0;
    u32 activePSOs = 0;
    u32 cachedPSOs = 0;
    u32 hits = 0;           // 缓存命中次数
    u32 misses = 0;         // 缓存未命中次数
    double hitRate = 0.0;   // 命中率
    u64 memoryUsage = 0;    // 内存使用量
    u32 compileQueueSize = 0;
    u32 compilingCount = 0;
};

/**
 * PSO缓存条目信息
 */
struct PSOCacheEntry {
    PipelineHandle handle;
    const char* debugName = nullptr;
    PSOCompileStatus status = PSOCompileStatus::Pending;
    u64 cacheKey = 0;
    u32 hitCount = 0;
    u64 lastUsedTime = 0;
    u64 memorySize = 0;
    bool isOnDisk = false;
};

/**
 * PSO缓存配置
 */
struct PipelineStateCacheConfig {
    // 缓存策略
    PSOCacheStrategy cacheStrategy = PSOCacheStrategy::MemoryAndDisk;
    
    // 内存限制
    u64 maxMemorySize = 256 * 1024 * 1024;  // 256MB
    u32 maxPSOCount = 1024;
    
    // 磁盘缓存
    const char* cacheFilePath = "cache/pso_cache.bin";
    bool compressDiskCache = true;
    
    // 异步编译
    u32 asyncCompileThreads = 2;
    u32 maxConcurrentCompiles = 4;
    
    // 预热设置
    bool autoWarmupOnLoad = true;
    PSOPriority defaultWarmupPriority = PSOPriority::Low;
    
    // 淘汰策略
    u32 maxUnusedFrames = 300;  // 5秒（假设60fps）
    bool enableLRU = true;
};

/**
 * Pipeline State Object 缓存接口
 * 
 * 提供PSO的缓存、异步编译、预热等功能，减少运行时卡顿
 * 
 * 使用示例：
 * @code
 * // 1. 创建缓存
 * auto cache = device->createPipelineStateCache(config);
 * 
 * // 2. 从磁盘加载缓存
 * cache->loadFromDisk("pso_cache.bin");
 * 
 * // 3. 预热常用PSO（避免运行时卡顿）
 * std::vector<GraphicsPipelineDesc> commonPSOs = collectCommonPSOs();
 * cache->warmupPSOs(commonPSOs, PSOPriority::High);
 * 
 * // 4. 获取或创建PSO（自动缓存）
 * PipelineHandle pso = cache->getOrCreatePSO(desc, "MyPSO");
 * 
 * // 5. 异步编译（不阻塞）
 * cache->compileAsync(desc, PSOPriority::Normal, 
 *     [](PipelineHandle handle, PSOCompileStatus status) {
 *         if (status == PSOCompileStatus::Completed) {
 *             LOG_INFO("PSO compiled successfully");
 *         }
 *     });
 * 
 * // 6. 运行时保存缓存
 * cache->saveToDisk("pso_cache.bin");
 * @endcode
 */
class IPipelineStateCache {
public:
    virtual ~IPipelineStateCache() = default;

    // ==================== 初始化与配置 ====================
    
    /**
     * @brief 初始化缓存系统
     * @param pipelineManager 管线管理器
     * @param config 缓存配置
     * @return 是否初始化成功
     */
    virtual bool initialize(IPipelineManager* pipelineManager, 
                           const PipelineStateCacheConfig& config) = 0;
    
    /**
     * @brief 关闭缓存系统
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 更新配置
     */
    virtual void setConfig(const PipelineStateCacheConfig& config) = 0;
    
    /**
     * @brief 获取当前配置
     */
    [[nodiscard]] virtual const PipelineStateCacheConfig& getConfig() const = 0;

    // ==================== PSO获取（核心接口）====================
    
    /**
     * @brief 获取或创建图形PSO（自动缓存）
     * @param desc PSO描述
     * @param debugName 调试名称
     * @return PSO句柄，失败返回无效句柄
     * @note 如果PSO在缓存中，直接返回；否则创建并缓存
     */
    [[nodiscard]] virtual PipelineHandle getOrCreatePSO(
        const GraphicsPipelineDesc& desc, 
        const char* debugName = nullptr) = 0;
    
    /**
     * @brief 获取或创建计算PSO（自动缓存）
     */
    [[nodiscard]] virtual PipelineHandle getOrCreatePSO(
        const ComputePipelineDesc& desc,
        const char* debugName = nullptr) = 0;
    
    /**
     * @brief 异步获取或创建PSO
     * @param desc PSO描述
     * @param priority 编译优先级
     * @param callback 编译完成回调
     * @return 缓存条目句柄（用于查询状态）
     * @note 如果PSO已缓存，立即调用callback；否则加入编译队列
     */
    virtual u64 getOrCreatePSOAsync(
        const GraphicsPipelineDesc& desc,
        PSOPriority priority,
        std::function<void(PipelineHandle, PSOCompileStatus)> callback) = 0;

    // ==================== 异步编译 ====================
    
    /**
     * @brief 异步编译图形PSO
     * @param desc PSO描述
     * @param priority 编译优先级
     * @param callback 完成回调
     */
    virtual void compileAsync(
        const GraphicsPipelineDesc& desc,
        PSOPriority priority = PSOPriority::Normal,
        std::function<void(PipelineHandle, PSOCompileStatus)> callback = nullptr) = 0;
    
    /**
     * @brief 异步编译计算PSO
     */
    virtual void compileAsync(
        const ComputePipelineDesc& desc,
        PSOPriority priority = PSOPriority::Normal,
        std::function<void(PipelineHandle, PSOCompileStatus)> callback = nullptr) = 0;
    
    /**
     * @brief 批量异步编译
     * @param jobs 编译任务列表
     * @note 自动排序优先级，高优先级先编译
     */
    virtual void compileBatch(
        const std::vector<PSOCompileJob>& jobs) = 0;
    
    /**
     * @brief 等待所有编译完成
     * @param timeoutMs 超时时间（毫秒），0表示无限等待
     * @return 是否全部完成
     */
    virtual bool waitForAllCompiles(u64 timeoutMs = 0) = 0;
    
    /**
     * @brief 取消所有未开始的编译任务
     */
    virtual void cancelPendingCompiles() = 0;

    // ==================== PSO预热（关键功能）====================
    
    /**
     * @brief 预热单个图形PSO
     * @param desc PSO描述
     * @param priority 编译优先级
     * @return 是否成功（可能因内存不足失败）
     * @note 用于游戏启动时预编译常用PSO，避免运行时卡顿
     */
    virtual bool warmupPSO(
        const GraphicsPipelineDesc& desc,
        PSOPriority priority = PSOPriority::Normal) = 0;
    
    /**
     * @brief 预热单个计算PSO
     */
    virtual bool warmupPSO(
        const ComputePipelineDesc& desc,
        PSOPriority priority = PSOPriority::Normal) = 0;
    
    /**
     * @brief 批量预热PSO
     * @param descs PSO描述列表
     * @param priority 编译优先级
     * @param progressCallback 进度回调（已编译数量，总数）
     * @return 预热统计
     */
    virtual PSOWarmupStats warmupPSOs(
        const std::vector<GraphicsPipelineDesc>& descs,
        PSOPriority priority = PSOPriority::Normal,
        std::function<void(u32, u32)> progressCallback = nullptr) = 0;
    
    /**
     * @brief 从材质收集PSO并预热
     * @param materials 材质列表
     * @param renderPasses 渲染通道列表（用于生成所有PSO变体）
     * @note 自动遍历材质的所有可能渲染状态组合
     */
    virtual PSOWarmupStats warmupFromMaterials(
        const std::vector<class IMaterial*>& materials,
        const std::vector<RenderPassHandle>& renderPasses) = 0;

    // ==================== 磁盘缓存 ====================
    
    /**
     * @brief 从磁盘加载缓存
     * @param filename 缓存文件路径
     * @return 是否成功
     * @note 通常在启动时调用，快速恢复上次运行的PSO
     */
    virtual bool loadFromDisk(const char* filename) = 0;
    
    /**
     * @brief 保存到磁盘
     * @param filename 缓存文件路径
     * @return 是否成功
     * @note 通常在退出时调用，保存已编译的PSO
     */
    virtual bool saveToDisk(const char* filename) const = 0;
    
    /**
     * @brief 导出PSO预热列表（用于构建时生成）
     * @param filename 输出文件
     * @note 运行时收集的PSO列表，可用于构建时预热
     */
    virtual void exportWarmupList(const char* filename) const = 0;
    
    /**
     * @brief 从预热列表加载
     * @param filename 列表文件
     * @param autoWarmup 是否立即开始预热
     */
    virtual bool importWarmupList(const char* filename, bool autoWarmup = true) = 0;

    // ==================== 缓存管理 ====================
    
    /**
     * @brief 获取PSO编译状态
     */
    [[nodiscard]] virtual PSOCompileStatus getPSOStatus(PipelineHandle handle) const = 0;
    
    /**
     * @brief 获取PSO编译状态（通过缓存键）
     */
    [[nodiscard]] virtual PSOCompileStatus getPSOStatusByKey(u64 cacheKey) const = 0;
    
    /**
     * @brief 检查PSO是否在缓存中
     */
    [[nodiscard]] virtual bool isPSOCached(const GraphicsPipelineDesc& desc) const = 0;
    
    /**
     * @brief 手动添加PSO到缓存
     * @param handle 已创建的PSO
     * @param desc 创建描述
     * @param debugName 调试名称
     * @note 用于手动管理缓存，通常由getOrCreatePSO自动处理
     */
    virtual void addToCache(PipelineHandle handle, 
                           const GraphicsPipelineDesc& desc,
                           const char* debugName = nullptr) = 0;
    
    /**
     * @brief 从缓存移除PSO
     */
    virtual void removeFromCache(PipelineHandle handle) = 0;
    
    /**
     * @brief 清空内存缓存
     * @param keepUsed 是否保留最近使用的
     */
    virtual void clearCache(bool keepUsed = true) = 0;
    
    /**
     * @brief 获取所有缓存条目
     */
    [[nodiscard]] virtual std::vector<PSOCacheEntry> getCacheEntries() const = 0;

    // ==================== 统计与查询 ====================
    
    /**
     * @brief 获取运行时统计
     */
    [[nodiscard]] virtual PSORuntimeStats getStats() const = 0;
    
    /**
     * @brief 获取当前编译队列大小
     */
    [[nodiscard]] virtual u32 getCompileQueueSize() const = 0;
    
    /**
     * @brief 获取正在编译的数量
     */
    [[nodiscard]] virtual u32 getCompilingCount() const = 0;
    
    /**
     * @brief 导出统计报告
     */
    virtual void dumpStats(const char* filename) const = 0;
    
    /**
     * @brief 打印缓存内容（调试用）
     */
    virtual void printCacheContents() const = 0;

    // ==================== 实时调整 ====================
    
    /**
     * @brief 暂停异步编译
     */
    virtual void pauseAsyncCompile() = 0;
    
    /**
     * @brief 恢复异步编译
     */
    virtual void resumeAsyncCompile() = 0;
    
    /**
     * @brief 设置最大并发编译数
     */
    virtual void setMaxConcurrentCompiles(u32 count) = 0;
    
    /**
     * @brief 设置内存限制
     */
    virtual void setMemoryLimit(u64 maxMemory) = 0;
};

/**
 * @brief PSO缓存工厂
 */
class PipelineStateCacheFactory {
public:
    /**
     * @brief 创建PSO缓存实例
     */
    static std::unique_ptr<IPipelineStateCache> create();
    
    /**
     * @brief 创建带配置的缓存实例
     */
    static std::unique_ptr<IPipelineStateCache> create(
        const PipelineStateCacheConfig& config);
};

/**
 * @brief PSO编译作用域守卫
 * 
 * 用于确保在作用域结束时等待编译完成
 */
class PSOCompileGuard {
public:
    explicit PSOCompileGuard(IPipelineStateCache* cache, u64 timeoutMs = 0)
        : cache_(cache), timeout_(timeoutMs) {}
    
    ~PSOCompileGuard() {
        if (cache_) {
            cache_->waitForAllCompiles(timeout_);
        }
    }
    
    // 禁用拷贝和移动
    PSOCompileGuard(const PSOCompileGuard&) = delete;
    PSOCompileGuard& operator=(const PSOCompileGuard&) = delete;
    PSOCompileGuard(PSOCompileGuard&&) = delete;
    PSOCompileGuard& operator=(PSOCompileGuard&&) = delete;
    
private:
    IPipelineStateCache* cache_;
    u64 timeout_;
};

// ==================== 便捷宏 ====================

/**
 * @brief 作用域PSO编译等待
 */
#define PSO_COMPILE_GUARD(cache, timeout) \
    render::PSOCompileGuard _psoGuard_##__LINE__(cache, timeout)

/**
 * @brief 快速获取PSO（带调试名）
 */
#define GET_PSO(cache, desc, name) \
    (cache)->getOrCreatePSO((desc), (name))

/**
 * @brief 预热PSO集合
 */
#define WARMUP_PSOS(cache, descs, priority) \
    do { \
        LOG_INFO("Warming up {} PSOs...", (descs).size()); \
        auto stats = (cache)->warmupPSOs((descs), (priority)); \
        LOG_INFO("PSO warmup complete: {}/{} compiled", \
            stats.compiledPSOs, stats.totalPSOs); \
    } while(0)

} // namespace render
