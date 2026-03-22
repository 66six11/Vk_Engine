#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class ICommandBuffer;
class IRenderPassContext;

/**
 * @brief 渲染图标志
 * @details RenderGraph全局配置选项
 */
enum class RenderGraphFlags : u32 {
    None = 0,                       ///< 无标志
    EnableAsyncCompute = 1 << 0,    ///< 启用异步计算队列
    EnableAsyncTransfer = 1 << 1,   ///< 启用异步传输队列
    EnableProfiling = 1 << 2,       ///< 启用性能分析
    EnableResourceAliasing = 1 << 3,///< 启用资源别名（内存复用）
    EnableBarrierBatching = 1 << 4, ///< 启用屏障批处理优化
    EnablePassMerging = 1 << 5,     ///< 启用通道合并优化
};

inline RenderGraphFlags operator|(RenderGraphFlags a, RenderGraphFlags b) {
    return static_cast<RenderGraphFlags>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline RenderGraphFlags operator&(RenderGraphFlags a, RenderGraphFlags b) {
    return static_cast<RenderGraphFlags>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(RenderGraphFlags flags, RenderGraphFlags flag) {
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

/**
 * @brief 渲染通道类型
 * @details 区分不同类型的渲染通道
 */
enum class RenderPassType {
    Graphics,   ///< 图形渲染通道
    Compute,    ///< 计算通道
    Transfer,   ///< 传输/复制通道
    RayTracing, ///< 光线追踪通道
    Custom      ///< 自定义通道
};

/**
 * @brief 渲染通道标志
 * @details 单个渲染通道的配置选项
 */
enum class RenderPassFlags : u32 {
    None = 0,               ///< 无标志
    AsyncCompute = 1 << 0,  ///< 在异步计算队列执行
    AsyncTransfer = 1 << 1, ///< 在异步传输队列执行
    SkipCull = 1 << 2,      ///< 跳过剔除检查
    SkipProfiling = 1 << 3, ///< 跳过性能分析
    SkipBarriers = 1 << 4,  ///< 跳过自动屏障（手动管理）
    External = 1 << 5,      ///< 外部通道（与外部API交互）
};

inline RenderPassFlags operator|(RenderPassFlags a, RenderPassFlags b) {
    return static_cast<RenderPassFlags>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline RenderPassFlags operator&(RenderPassFlags a, RenderPassFlags b) {
    return static_cast<RenderPassFlags>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief 资源访问类型
 * @details 定义对资源的访问模式
 */
enum class ResourceAccess {
    Read,       ///< 只读访问
    Write,      ///< 只写访问
    ReadWrite   ///< 读写访问
};

/**
 * 资源使用描述
 */
struct RenderGraphResourceUsage {
    VersionedResourceHandle handle;
    ResourceAccess access = ResourceAccess::Read;
    ResourceState requiredState = ResourceState::Undefined;
    SubresourceRange subresourceRange;
};

/**
 * 渲染图描述
 */
struct RenderGraphDesc {
    const char* name = "RenderGraph";
    RenderGraphFlags flags = RenderGraphFlags::EnableResourceAliasing |
                             RenderGraphFlags::EnableBarrierBatching;
    u32 maxPasses = 128;
    u32 maxResources = 256;
    u32 maxTransientMemory = 256 * 1024 * 1024; // 256MB
};

/**
 * 渲染通道描述
 */
struct RenderPassDesc {
    const char* name = "RenderPass";
    RenderPassType type = RenderPassType::Graphics;
    RenderPassFlags flags = RenderPassFlags::None;

    // 执行队列配置
    QueueType queueType = QueueType::Graphics;  // 在哪个队列上执行
    bool asyncCompute = false;                   // 是否在异步计算队列执行
    bool asyncTransfer = false;                  // 是否在异步传输队列执行

    // 队列同步点
    std::vector<QueueSyncPoint> queueSyncPoints;

    // Subpass配置（用于Subpass合并优化）
    u32 subpassIndex = 0;                        // 所属Subpass索引
    bool mergeWithNext = false;                  // 是否可与下一个Pass合并

    // 输入资源（读取）
    std::vector<RenderGraphResourceUsage> inputs;

    // 输出资源（写入）
    std::vector<RenderGraphResourceUsage> outputs;

    // 渲染目标（仅图形通道）
    std::vector<VersionedTextureHandle> colorTargets;
    VersionedTextureHandle depthStencilTarget;
    ClearValue clearColor = ClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    ClearValue clearDepthStencil = ClearValue::depthStencil(1.0f, 0);
    bool clearColorEnabled = false;
    bool clearDepthEnabled = false;
    bool clearStencilEnabled = false;

    // 执行回调
    using ExecuteCallback = std::function<void(IRenderPassContext&)>;
    ExecuteCallback executeCallback;

    // 设置回调（可选，用于动态配置）
    using SetupCallback = std::function<void(IRenderPassContext&)>;
    SetupCallback setupCallback;

    // 清理回调（可选）
    using TeardownCallback = std::function<void(IRenderPassContext&)>;
    TeardownCallback teardownCallback;
};

/**
 * 渲染图资源描述（瞬态资源）
 */
struct RenderGraphResourceDesc {
    const char* name = nullptr;
    ResourceType type = ResourceType::Texture2D;
    Format format = Format::RGBA8_UNORM;
    ResourceExtent extent = {1, 1, 1};
    u32 mipLevels = 1;
    u32 arrayLayers = 1;
    ResourceUsage usage = ResourceUsage::ShaderResource;
    ClearValue clearValue = {};
    bool isImported = false;
    ResourceHandle importedResource;
};

/**
 * 编译结果
 */
struct RenderGraphCompileResult {
    bool success = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    // 执行顺序
    std::vector<RenderGraphPassHandle> executionOrder;

    // 资源分配
    u64 totalMemory = 0;
    u64 peakMemory = 0;
    u32 aliasedResourceCount = 0;

    // 性能估计
    u32 estimatedPasses = 0;
    u32 estimatedBarriers = 0;
};

/**
 * 执行结果
 */
struct RenderGraphExecuteResult {
    bool success = false;
    std::string error;
    double gpuTimeMs = 0.0;
    u32 executedPasses = 0;
};

/**
 * 渲染图信息
 */
struct RenderGraphInfo {
    const char* name = nullptr;
    u32 passCount = 0;
    u32 resourceCount = 0;
    u32 transientResourceCount = 0;
    u64 transientMemoryUsage = 0;
    RenderGraphFlags flags;
};

/**
 * 渲染通道上下文
 * 在渲染通道执行时提供资源访问和命令录制功能
 */
class IRenderPassContext {
public:
    virtual ~IRenderPassContext() = default;

    // 当前通道信息
    virtual RenderGraphPassHandle getCurrentPass() const = 0;
    virtual const char* getPassName() const = 0;
    virtual RenderPassType getPassType() const = 0;

    // 命令缓冲区访问
    virtual ICommandBuffer* getCommandBuffer() = 0;

    // 资源访问
    virtual TextureHandle getTexture(VersionedTextureHandle handle) = 0;
    virtual BufferHandle getBuffer(VersionedBufferHandle handle) = 0;
    virtual ResourceViewHandle getTextureView(VersionedTextureHandle handle,
                                             const TextureViewDesc& desc = {}) = 0;
    virtual ResourceViewHandle getBufferView(VersionedBufferHandle handle,
                                            const BufferViewDesc& desc = {}) = 0;

    // 管线绑定
    virtual void setPipeline(PipelineHandle pipeline) = 0;
    virtual void setDescriptorSet(u32 setIndex, DescriptorSetHandle descriptorSet) = 0;
    virtual void setPushConstants(ShaderStage stages, u32 offset, u32 size, const void* data) = 0;

    // 渲染状态
    virtual void setViewport(const Viewport& viewport) = 0;
    virtual void setViewports(const std::vector<Viewport>& viewports) = 0;
    virtual void setScissor(const Rect& scissor) = 0;
    virtual void setScissors(const std::vector<Rect>& scissors) = 0;
    virtual void setBlendConstants(const std::array<float, 4>& constants) = 0;
    virtual void setStencilReference(u8 reference) = 0;
    virtual void setDepthBounds(float minBounds, float maxBounds) = 0;

    // 绘制命令
    virtual void draw(u32 vertexCount, u32 instanceCount = 1,
                     u32 firstVertex = 0, u32 firstInstance = 0) = 0;
    virtual void drawIndexed(u32 indexCount, u32 instanceCount = 1,
                            u32 firstIndex = 0, i32 vertexOffset = 0,
                            u32 firstInstance = 0) = 0;
    virtual void drawIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) = 0;
    virtual void drawIndexedIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) = 0;

    // 计算命令
    virtual void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) = 0;
    virtual void dispatchIndirect(BufferHandle buffer, u64 offset) = 0;

    // 光追命令
    virtual void traceRays(u32 width, u32 height, u32 depth = 1) = 0;

    // 资源屏障（通常由Render Graph自动管理）
    virtual void resourceBarrier(TextureHandle texture,
                                ResourceState newState,
                                const SubresourceRange& range = {}) = 0;
    virtual void resourceBarrier(BufferHandle buffer, ResourceState newState) = 0;

    // 查询
    virtual void beginQuery(QueryPoolHandle pool, u32 query) = 0;
    virtual void endQuery(QueryPoolHandle pool, u32 query) = 0;
    virtual void writeTimestamp(QueryPoolHandle pool, u32 query) = 0;

    // 调试标记
    virtual void beginDebugMarker(const char* name, const Color& color) = 0;
    virtual void endDebugMarker() = 0;
    virtual void insertDebugMarker(const char* name, const Color& color) = 0;
};

/**
 * 渲染图构建器
 * 用于声明渲染通道和资源
 */
class IRenderGraphBuilder {
public:
    virtual ~IRenderGraphBuilder() = default;

    // 创建瞬态纹理
    virtual VersionedTextureHandle createTexture(const char* name,
                                                const RenderGraphResourceDesc& desc) = 0;

    // 创建瞬态缓冲区
    virtual VersionedBufferHandle createBuffer(const char* name,
                                              u64 size,
                                              BufferType type = BufferType::Storage,
                                              MemoryType memoryType = MemoryType::Default) = 0;

    // 导入外部资源
    virtual VersionedTextureHandle importTexture(const char* name,
                                                TextureHandle texture,
                                                ResourceState initialState) = 0;
    virtual VersionedBufferHandle importBuffer(const char* name,
                                              BufferHandle buffer,
                                              ResourceState initialState) = 0;

    // 获取已声明资源
    virtual VersionedTextureHandle getTexture(const char* name) = 0;
    virtual VersionedBufferHandle getBuffer(const char* name) = 0;

    // 添加渲染通道
    virtual RenderGraphPassHandle addPass(const RenderPassDesc& desc) = 0;

    // 读取资源
    virtual void readTexture(RenderGraphPassHandle pass,
                            VersionedTextureHandle texture,
                            ResourceState state = ResourceState::ShaderResource) = 0;
    virtual void readBuffer(RenderGraphPassHandle pass,
                           VersionedBufferHandle buffer,
                           ResourceState state = ResourceState::ShaderResource) = 0;

    // 写入资源（创建新版本）
    virtual VersionedTextureHandle writeTexture(RenderGraphPassHandle pass,
                                               const char* name,
                                               VersionedTextureHandle texture,
                                               ResourceState state = ResourceState::RenderTarget) = 0;
    virtual VersionedBufferHandle writeBuffer(RenderGraphPassHandle pass,
                                             const char* name,
                                             VersionedBufferHandle buffer,
                                             ResourceState state = ResourceState::UnorderedAccess) = 0;

    // 创建渲染目标
    virtual VersionedTextureHandle createRenderTarget(const char* name,
                                                     const RenderGraphResourceDesc& desc) = 0;

    // 设置输出
    virtual void setOutput(VersionedTextureHandle texture) = 0;

    // 队列同步支持
    // 在指定Pass后插入队列同步点
    virtual void queueSync(RenderGraphPassHandle pass,
                          QueueType srcQueue,
                          QueueType dstQueue) = 0;

    // 标记下一个Pass在异步计算队列执行
    virtual void executeNextAsync(QueueType queueType = QueueType::Compute) = 0;

    // Subpass合并支持
    // 开始一个Subpass组，后续Pass可能合并为一个RenderPass
    virtual void beginSubpassGroup(const char* name) = 0;
    virtual void endSubpassGroup() = 0;
};

/**
 * 渲染图管理器接口
 * 负责渲染图的编译和执行
 */
class IRenderGraphManager {
public:
    virtual ~IRenderGraphManager() = default;

    // 渲染图生命周期
    virtual RenderGraphHandle createRenderGraph(const RenderGraphDesc& desc) = 0;
    virtual void destroyRenderGraph(RenderGraphHandle handle) = 0;
    virtual RenderGraphInfo getGraphInfo(RenderGraphHandle handle) const = 0;

    // 构建渲染图
    virtual IRenderGraphBuilder* beginBuild(RenderGraphHandle handle) = 0;
    virtual RenderGraphCompileResult endBuild(RenderGraphHandle handle) = 0;

    // 编译
    virtual RenderGraphCompileResult compile(RenderGraphHandle handle) = 0;
    virtual void invalidateCompilation(RenderGraphHandle handle) = 0;

    // 执行
    virtual RenderGraphExecuteResult execute(RenderGraphHandle handle) = 0;
    virtual RenderGraphExecuteResult execute(RenderGraphHandle handle,
                                            ICommandBuffer* commandBuffer) = 0;

    // 调试与可视化
    virtual void dumpGraph(RenderGraphHandle handle, const char* filename) = 0;
    virtual void visualizeGraph(RenderGraphHandle handle) = 0;

    // 缓存管理
    virtual void clearCompilationCache() = 0;
    virtual void warmupCache(RenderGraphHandle handle) = 0;

    // 统计
    virtual u32 getActiveGraphCount() const = 0;
    virtual u64 getTotalTransientMemory() const = 0;

    // 队列配置
    // 配置渲染图使用的队列族索引
    virtual void setQueueFamilyIndex(QueueType queueType, u32 familyIndex) = 0;
    virtual u32 getQueueFamilyIndex(QueueType queueType) const = 0;

    // Split Barrier支持
    // 启用Split Barrier优化（适用于长时间计算任务）
    virtual void enableSplitBarriers(bool enable) = 0;
    virtual bool isSplitBarriersEnabled() const = 0;

    // Subpass合并配置
    virtual void enableSubpassMerging(bool enable) = 0;
    virtual bool isSubpassMergingEnabled() const = 0;

    // 延迟资源分配
    virtual void enableLazyResourceAllocation(bool enable) = 0;
    virtual bool isLazyResourceAllocationEnabled() const = 0;

    // 资源视图缓存
    virtual void enableResourceViewCaching(bool enable) = 0;
    virtual bool isResourceViewCachingEnabled() const = 0;
};

} // namespace render