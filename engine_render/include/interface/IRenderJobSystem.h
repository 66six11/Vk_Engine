#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include <functional>
#include <future>

namespace render {

// 前向声明
class ICommandBuffer;
class ICommandPool;
class IRenderDevice;

/**
 * @brief 渲染任务优先级
 * @details 多线程渲染任务的调度优先级
 */
enum class JobPriority : u8 {
    Low = 0,        ///< 低优先级（后台任务）
    Normal = 1,     ///< 正常优先级（默认）
    High = 2,       ///< 高优先级
    Critical = 3    ///< 关键优先级（阻塞渲染管线）
};

/**
 * @brief 渲染任务类型
 * @details 区分不同类型的渲染作业
 */
enum class JobType : u8 {
    Culling,        ///< 视锥剔除任务
    CommandRecord,  ///< 命令缓冲区录制任务
    Compute,        ///< 计算着色器任务
    Upload,         ///< GPU数据上传任务
    BuildBVH,       ///< 加速结构构建任务
    Custom          ///< 自定义任务类型
};

/**
 * 任务统计
 */
struct JobStats {
    u32 totalJobs = 0;
    u32 pendingJobs = 0;
    u32 runningJobs = 0;
    u32 completedJobs = 0;
    u32 failedJobs = 0;
    double averageJobTimeMs = 0.0;
    u32 workerThreadCount = 0;
    u32 activeThreadCount = 0;
};

/**
 * 任务描述
 */
struct JobDesc {
    const char* name = nullptr;
    JobType type = JobType::Custom;
    JobPriority priority = JobPriority::Normal;
    std::function<void()> task;
    std::vector<JobHandle> dependencies;  // 依赖的任务
    bool allowParallel = true;            // 是否允许并行
};

/**
 * 并行渲染任务描述
 */
struct ParallelRenderJobDesc {
    const char* name = nullptr;
    u32 jobCount = 1;                     // 并行任务数量
    std::function<void(u32 jobIndex, ICommandBuffer* cmdBuffer)> task;  // jobIndex: 0..jobCount-1
    JobPriority priority = JobPriority::Normal;
    std::vector<RenderPassHandle> renderPasses;  // 关联的渲染通道
};

/**
 * 任务句柄
 */
using JobHandle = HandleType<JobTag>;

/**
 * 任务结果
 */
template<typename T>
using JobResult = std::future<T>;

/**
 * 工作线程配置
 */
struct WorkerThreadConfig {
    u32 threadCount = 0;                  // 0 = 自动（硬件并发数-1）
    u32 commandPoolsPerThread = 2;        // 每线程命令池数量
    bool pinThreadsToCores = false;       // 是否绑定到特定核心
    u32 threadStackSize = 1024 * 1024;    // 线程栈大小（1MB）
    const char* threadNamePrefix = "RenderWorker";
};

/**
 * 任务系统配置
 */
struct JobSystemConfig {
    WorkerThreadConfig workerConfig;
    u32 maxPendingJobs = 1024;            // 最大等待任务数
    u32 maxRunningJobs = 64;              // 最大并发任务数
    bool enableWorkStealing = true;       // 启用工作窃取
    bool enableLoadBalancing = true;      // 启用负载均衡
    u32 jobQueueSize = 256;               // 每线程队列大小
};

/**
 * 多线程渲染任务系统接口
 * 
 * 提供并行渲染任务、工作线程管理、命令缓冲区池等功能
 * 
 * 使用示例：
 * @code
 * // 1. 创建任务系统
 * auto jobSystem = render::JobSystemFactory::create(config);
 * jobSystem->initialize(renderDevice);
 * 
 * // 2. 提交简单任务
 * auto handle = jobSystem->submit({
 *     .name = "BuildBVH",
 *     .type = JobType::BuildBVH,
 *     .priority = JobPriority::High,
 *     .task = [&]() { sceneMgr->buildBVH(); }
 * });
 * 
 * // 3. 等待完成
 * jobSystem->waitForJob(handle);
 * 
 * // 4. 并行命令录制（多线程渲染核心）
 * jobSystem->dispatchParallelRenderJobs({
 *     .jobCount = 4,  // 4个并行任务
 *     .task = [](u32 jobIndex, ICommandBuffer* cmdBuffer) {
 *         // 每个线程录制自己的命令缓冲区
 *         for (u32 i = jobIndex; i < objectCount; i += 4) {
 *             renderObject(objects[i], cmdBuffer);
 *         }
 *     }
 * });
 * 
 * // 5. 获取所有命令缓冲区并提交
 * auto cmdBuffers = jobSystem->getRecordedCommandBuffers();
 * graphicsQueue->submit(cmdBuffers.size(), cmdBuffers.data());
 * @endcode
 */
class IRenderJobSystem {
public:
    virtual ~IRenderJobSystem() = default;

    // ==================== 初始化与配置 ====================
    
    /**
     * @brief 初始化任务系统
     * @param device 渲染设备
     * @param config 配置
     */
    virtual bool initialize(IRenderDevice* device, const JobSystemConfig& config) = 0;
    
    /**
     * @brief 关闭任务系统
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 更新配置
     */
    virtual void setConfig(const JobSystemConfig& config) = 0;
    
    /**
     * @brief 获取当前配置
     */
    [[nodiscard]] virtual const JobSystemConfig& getConfig() const = 0;

    // ==================== 任务提交（核心接口）====================
    
    /**
     * @brief 提交异步任务
     * @param desc 任务描述
     * @return 任务句柄（用于查询状态或等待）
     */
    [[nodiscard]] virtual JobHandle submit(const JobDesc& desc) = 0;
    
    /**
     * @brief 提交带返回值的任务
     * @tparam T 返回值类型
     * @param desc 任务描述
     * @param result 返回值future
     * @return 任务句柄
     */
    template<typename T>
    [[nodiscard]] JobHandle submitWithResult(const JobDesc& desc, JobResult<T>& result) {
        std::promise<T> promise;
        result = promise.get_future();
        
        JobDesc modifiedDesc = desc;
        auto originalTask = desc.task;
        modifiedDesc.task = [originalTask, &promise]() {
            originalTask();
            promise.set_value(T{});  // 简化版本，实际需要根据task返回值
        };
        
        return submit(modifiedDesc);
    }
    
    /**
     * @brief 批量提交任务
     * @param jobs 任务描述列表
     * @return 任务句柄列表
     * @note 自动排序优先级，高优先级先执行
     */
    [[nodiscard]] virtual std::vector<JobHandle> submitBatch(
        const std::vector<JobDesc>& jobs) = 0;
    
    /**
     * @brief 提交依赖任务（等待依赖完成后执行）
     * @param desc 任务描述
     * @param dependencies 依赖的任务句柄
     * @return 任务句柄
     */
    [[nodiscard]] virtual JobHandle submitDependent(
        const JobDesc& desc,
        const std::vector<JobHandle>& dependencies) = 0;

    // ==================== 并行渲染（核心功能）====================
    
    /**
     * @brief 派发并行渲染任务
     * @param desc 并行渲染任务描述
     * @return 任务句柄（代表整个并行任务组）
     * @note 每个任务获得独立的命令缓冲区和命令池
     */
    [[nodiscard]] virtual JobHandle dispatchParallelRenderJobs(
        const ParallelRenderJobDesc& desc) = 0;
    
    // ==================== 命令缓冲区管理 ====================
    
    /**
     * @brief 获取线程本地命令缓冲区
     * @param threadIndex 线程索引
     * @param level 命令缓冲区级别
     * @return 命令缓冲区指针
     * @note 每个线程有独立的命令池，无需同步
     */
    [[nodiscard]] virtual ICommandBuffer* getThreadCommandBuffer(
        u32 threadIndex,
        CommandBufferLevel level = CommandBufferLevel::Secondary) = 0;
    
    /**
     * @brief 获取所有录制的命令缓冲区
     * @return 命令缓冲区列表
     * @note 并行渲染任务完成后调用
     */
    [[nodiscard]] virtual std::vector<ICommandBuffer*> getRecordedCommandBuffers() = 0;
    
    /**
     * @brief 重置所有线程的命令池
     * @note 每帧开始时调用
     */
    virtual void resetCommandPools() = 0;
    
    /**
     * @brief 获取线程数量
     */
    [[nodiscard]] virtual u32 getThreadCount() const = 0;
    
    /**
     * @brief 获取当前线程索引（0 = 主线程）
     */
    [[nodiscard]] virtual u32 getCurrentThreadIndex() const = 0;
    
    /**
     * @brief 检查是否在工作者线程上
     */
    [[nodiscard]] virtual bool isWorkerThread() const = 0;

    // ==================== 任务同步 ====================
    
    /**
     * @brief 等待单个任务完成
     * @param handle 任务句柄
     * @param timeoutMs 超时（毫秒，0 = 无限）
     * @return 是否成功完成
     */
    virtual bool waitForJob(JobHandle handle, u64 timeoutMs = 0) = 0;
    
    /**
     * @brief 等待多个任务完成
     */
    virtual bool waitForJobs(const std::vector<JobHandle>& handles, 
                            u64 timeoutMs = 0) = 0;
    
    /**
     * @brief 等待所有任务完成
     */
    virtual void waitForAllJobs() = 0;
    
    /**
     * @brief 创建同步点（屏障）
     * @return 屏障任务句柄，所有之前提交的任务都完成后才执行之后的任务
     */
    [[nodiscard]] virtual JobHandle createBarrier() = 0;
    
    /**
     * @brief 检查任务是否完成
     */
    [[nodiscard]] virtual bool isJobCompleted(JobHandle handle) const = 0;
    
    /**
     * @brief 检查任务是否失败
     */
    [[nodiscard]] virtual bool isJobFailed(JobHandle handle) const = 0;

    // ==================== 任务查询 ====================
    
    /**
     * @brief 获取任务统计
     */
    [[nodiscard]] virtual JobStats getStats() const = 0;
    
    /**
     * @brief 获取任务执行时间
     */
    [[nodiscard]] virtual double getJobExecutionTime(JobHandle handle) const = 0;
    
    /**
     * @brief 获取当前活跃任务数
     */
    [[nodiscard]] virtual u32 getActiveJobCount() const = 0;
    
    /**
     * @brief 获取等待队列大小
     */
    [[nodiscard]] virtual u32 getPendingJobCount() const = 0;

    // ==================== 工作线程控制 ====================
    
    /**
     * @brief 暂停所有工作线程
     */
    virtual void pauseWorkers() = 0;
    
    /**
     * @brief 恢复所有工作线程
     */
    virtual void resumeWorkers() = 0;
    
    /**
     * @brief 设置工作线程数量（动态调整）
     */
    virtual void setWorkerThreadCount(u32 count) = 0;
    
    /**
     * @brief 启用/禁用工作窃取
     */
    virtual void enableWorkStealing(bool enable) = 0;

    // ==================== 每帧更新 ====================
    
    /**
     * @brief 每帧开始
     * @note 重置命令池，清理已完成任务
     */
    virtual void beginFrame() = 0;
    
    /**
     * @brief 每帧结束
     * @note 等待本帧所有渲染任务完成
     */
    virtual void endFrame() = 0;

    // ==================== 调试与性能 ====================
    
    /**
     * @brief 导出任务执行时间线（Chrome Tracing格式）
     */
    virtual void exportTimeline(const char* filename) = 0;
    
    /**
     * @brief 打印任务统计
     */
    virtual void printStats() const = 0;
    
    /**
     * @brief 可视化任务依赖图
     */
    virtual void visualizeDependencyGraph(const char* filename) = 0;
};

/**
 * @brief 任务系统工厂
 */
class JobSystemFactory {
public:
    static std::unique_ptr<IRenderJobSystem> create();
    static std::unique_ptr<IRenderJobSystem> create(const JobSystemConfig& config);
};

// ==================== 便捷宏 ====================

/**
 * @brief 并行渲染作用域
 */
#define PARALLEL_RENDER_JOBS(jobSystem, count, taskBody) \
    (jobSystem)->dispatchParallelRenderJobs({ \
        .jobCount = static_cast<u32>(count), \
        .task = taskBody \
    })

/**
 * @brief 提交异步任务
 */
#define SUBMIT_JOB(jobSystem, jobType, priority, taskBody) \
    (jobSystem)->submit({ \
        .type = (jobType), \
        .priority = (priority), \
        .task = taskBody \
    })

/**
 * @brief 等待任务完成（带超时）
 */
#define WAIT_JOB(jobSystem, handle, timeout) \
    (jobSystem)->waitForJob((handle), (timeout))

/**
 * @brief 线程本地命令缓冲区
 */
#define THREAD_CMD_BUFFER(jobSystem, threadIndex) \
    (jobSystem)->getThreadCommandBuffer((threadIndex))

/**
 * @brief 作用域帧同步
 */
class JobSystemFrameScope {
public:
    explicit JobSystemFrameScope(IRenderJobSystem* system) 
        : jobSystem(system) {
        if (jobSystem) {
            jobSystem->beginFrame();
        }
    }
    
    ~JobSystemFrameScope() {
        if (jobSystem) {
            jobSystem->endFrame();
        }
    }
    
    JobSystemFrameScope(const JobSystemFrameScope&) = delete;
    JobSystemFrameScope& operator=(const JobSystemFrameScope&) = delete;
    JobSystemFrameScope(JobSystemFrameScope&&) = delete;
    JobSystemFrameScope& operator=(JobSystemFrameScope&&) = delete;
    
private:
    IRenderJobSystem* jobSystem;
};

#define JOB_SYSTEM_FRAME_SCOPE(jobSystem) \
    render::JobSystemFrameScope _jobFrameScope_##__LINE__(jobSystem)

} // namespace render
