#pragma once

#include "interface/IRenderGraph.h"

namespace render {

// 前向声明
class IResourceManager;
class IPipelineManager;
class ICommandBuffer;

namespace render_graph {

// 前向声明
class RenderGraphBuilder;
class RenderGraphCompiler;
class RenderGraphExecutor;
class ResourceAllocator;
class BarrierManager;

/**
 * 渲染图节点
 */
struct RenderPassNode {
    RenderGraphPassHandle handle;
    std::string name;
    RenderPassType type = RenderPassType::Graphics;
    RenderPassFlags flags = RenderPassFlags::None;

    // 队列配置
    QueueType queueType = QueueType::Graphics;
    bool asyncCompute = false;
    bool asyncTransfer = false;
    std::vector<QueueSyncPoint> queueSyncPoints;

    // Subpass配置
    u32 subpassIndex = 0;
    bool mergeWithNext = false;
    bool isSubpassMerged = false;  // 编译后设置

    // 资源引用
    std::vector<ResourceUsage> inputs;
    std::vector<ResourceUsage> outputs;

    // 渲染目标
    std::vector<VersionedTextureHandle> colorTargets;
    VersionedTextureHandle depthStencilTarget;
    ClearValue clearColor;
    ClearValue clearDepthStencil;
    bool clearColorEnabled = false;
    bool clearDepthEnabled = false;
    bool clearStencilEnabled = false;

    // 回调
    RenderPassDesc::ExecuteCallback executeCallback;
    RenderPassDesc::SetupCallback setupCallback;
    RenderPassDesc::TeardownCallback teardownCallback;

    // 编译信息
    u32 executionOrder = 0;
    std::vector<RenderGraphPassHandle> dependencies;
    bool culled = false;
    bool async = false;
};

/**
 * 资源节点
 */
struct ResourceNode {
    VersionedResourceHandle handle;
    std::string name;
    ResourceType type = ResourceType::Texture2D;

    // 资源描述
    RenderGraphResourceDesc desc;

    // 生命周期
    u32 firstUsePass = ~0u;
    u32 lastUsePass = 0;

    // 别名信息
    VersionedResourceHandle aliasedResource;
    bool isAlias = false;

    // 实际资源
    ResourceHandle actualResource;
    bool isImported = false;

    // 版本历史
    std::vector<std::pair<u32, ResourceState>> stateHistory;
};

/**
 * 资源分配信息
 */
struct ResourceAllocation {
    VersionedResourceHandle handle;
    u64 offset = 0;
    u64 size = 0;
    u32 firstUsePass = 0;
    u32 lastUsePass = 0;
    MemoryType memoryType = MemoryType::Default;

    // 延迟分配支持
    bool isAllocated = false;           // 是否已实际分配
    bool isLazyAllocation = false;      // 是否启用延迟分配

    // 别名信息
    bool isAliased = false;             // 是否使用别名内存
    VersionedResourceHandle aliasedWith; // 与哪个资源共享内存
    bool requiresAliasingBarrier = false; // 是否需要别名屏障

    // 实际资源（延迟绑定）
    ResourceHandle actualResource;
};

/**
 * 屏障信息
 */
struct BarrierInfo {
    VersionedResourceHandle resource;
    ResourceState oldState;
    ResourceState newState;
    u32 srcPass;
    u32 dstPass;
    PipelineStage srcStage;
    PipelineStage dstStage;
    AccessFlags srcAccess;
    AccessFlags dstAccess;
    SubresourceRange range;

    // Split Barrier支持
    bool isSplitBarrier = false;        // 是否是Split Barrier
    SplitBarrierPhase splitPhase = SplitBarrierPhase::Begin;
    u32 splitCompletionPass = ~0u;      // Split Barrier完成于哪个Pass

    // 别名屏障
    bool isAliasingBarrier = false;
    ResourceHandle beforeResource;      // 别名前的资源
    ResourceHandle afterResource;       // 别名后的资源

    // 队列间屏障
    bool isQueueTransfer = false;
    QueueType srcQueue = QueueType::Graphics;
    QueueType dstQueue = QueueType::Graphics;
};

/**
 * 渲染图实现
 */
class RenderGraph : public IRenderGraphManager {
public:
    RenderGraph(IResourceManager* resourceManager, IPipelineManager* pipelineManager);
    ~RenderGraph() override;

    bool initialize(const RenderGraphDesc& desc);
    void shutdown();

    // IRenderGraphManager接口实现
    RenderGraphHandle createRenderGraph(const RenderGraphDesc& desc) override;
    void destroyRenderGraph(RenderGraphHandle handle) override;
    RenderGraphInfo getGraphInfo(RenderGraphHandle handle) const override;

    IRenderGraphBuilder* beginBuild(RenderGraphHandle handle) override;
    RenderGraphCompileResult endBuild(RenderGraphHandle handle) override;

    RenderGraphCompileResult compile(RenderGraphHandle handle) override;
    void invalidateCompilation(RenderGraphHandle handle) override;

    RenderGraphExecuteResult execute(RenderGraphHandle handle) override;
    RenderGraphExecuteResult execute(RenderGraphHandle handle,
                                    ICommandBuffer* commandBuffer) override;

    void dumpGraph(RenderGraphHandle handle, const char* filename) override;
    void visualizeGraph(RenderGraphHandle handle) override;

    void clearCompilationCache() override;
    void warmupCache(RenderGraphHandle handle) override;

    u32 getActiveGraphCount() const override;
    u64 getTotalTransientMemory() const override;

    // 内部方法（供builder使用）
    VersionedTextureHandle createTextureInternal(const char* name,
                                                const RenderGraphResourceDesc& desc);
    VersionedBufferHandle createBufferInternal(const char* name,
                                              u64 size,
                                              BufferType type,
                                              MemoryType memoryType);
    VersionedTextureHandle importTextureInternal(const char* name,
                                                TextureHandle texture,
                                                ResourceState initialState);
    VersionedBufferHandle importBufferInternal(const char* name,
                                              BufferHandle buffer,
                                              ResourceState initialState);
    RenderGraphPassHandle addPassInternal(const RenderPassDesc& desc);

    // 获取节点
    RenderPassNode* getPassNode(RenderGraphPassHandle handle);
    ResourceNode* getResourceNode(VersionedResourceHandle handle);

private:
    IResourceManager* resourceManager;
    IPipelineManager* pipelineManager;

    // 渲染图数据
    RenderGraphDesc desc;
    std::string name;
    RenderGraphHandle handle;

    // 节点存储
    std::unordered_map<RenderGraphPassHandle, std::unique_ptr<RenderPassNode>> passes;
    std::unordered_map<Handle, std::unique_ptr<ResourceNode>> resources;

    // 句柄分配
    HandleAllocator<RenderGraphPassHandle> passHandleAllocator;
    Handle nextResourceIndex = 1;

    // 构建器
    std::unique_ptr<RenderGraphBuilder> builder;

    // 编译器
    std::unique_ptr<RenderGraphCompiler> compiler;

    // 执行器
    std::unique_ptr<RenderGraphExecutor> executor;

    // 资源分配器
    std::unique_ptr<ResourceAllocator> resourceAllocator;

    // 屏障管理器
    std::unique_ptr<BarrierManager> barrierManager;

    // 编译状态
    bool compiled = false;
    RenderGraphCompileResult compileResult;

    // 执行顺序
    std::vector<RenderGraphPassHandle> executionOrder;

    // 资源分配
    std::vector<ResourceAllocation> resourceAllocations;

    // 屏障列表
    std::vector<BarrierInfo> barriers;

    // 输出资源
    VersionedTextureHandle outputTexture;

    // 队列配置
    std::unordered_map<QueueType, u32> queueFamilyIndices;
    std::unordered_map<QueueType, ICommandQueue*> queues;

    // 高级功能标志
    bool enableSplitBarriers = false;
    bool enableSubpassMerging = false;
    bool enableLazyResourceAllocation = false;
    bool enableResourceViewCaching = false;

    // Subpass合并
    std::vector<std::vector<RenderGraphPassHandle>> subpassGroups;
    bool inSubpassGroup = false;
    std::string currentSubpassGroupName;

    // 资源视图缓存
    std::unordered_map<u64, ResourceViewHandle> resourceViewCache;

    // 辅助函数
    void reset();
    void cullUnusedPasses();
    void topologicalSort();
    void calculateResourceLifetimes();
    void allocateResources();
    void generateBarriers();
    void executePass(RenderPassNode* pass, IRenderPassContext* context);

    // 队列同步
    void insertQueueSyncBarrier(const QueueSyncPoint& syncPoint, u32 passIndex);
    void buildQueueDependencyGraph();

    // Subpass合并
    void findMergeablePasses();
    void buildSubpassGroups();

    // 延迟分配
    void allocateResourceLazy(VersionedResourceHandle handle);

    // 资源视图缓存
    ResourceViewHandle getCachedResourceView(VersionedResourceHandle handle,
                                             const TextureViewDesc& desc);
    void clearResourceViewCache();
};

/**
 * 渲染图构建器实现
 */
class RenderGraphBuilder : public IRenderGraphBuilder {
public:
    explicit RenderGraphBuilder(RenderGraph* graph);
    ~RenderGraphBuilder() override = default;

    // IRenderGraphBuilder接口实现
    VersionedTextureHandle createTexture(const char* name,
                                        const RenderGraphResourceDesc& desc) override;
    VersionedBufferHandle createBuffer(const char* name,
                                      u64 size,
                                      BufferType type,
                                      MemoryType memoryType) override;

    VersionedTextureHandle importTexture(const char* name,
                                        TextureHandle texture,
                                        ResourceState initialState) override;
    VersionedBufferHandle importBuffer(const char* name,
                                      BufferHandle buffer,
                                      ResourceState initialState) override;

    VersionedTextureHandle getTexture(const char* name) override;
    VersionedBufferHandle getBuffer(const char* name) override;

    RenderGraphPassHandle addPass(const RenderPassDesc& desc) override;

    void readTexture(RenderGraphPassHandle pass,
                    VersionedTextureHandle texture,
                    ResourceState state) override;
    void readBuffer(RenderGraphPassHandle pass,
                   VersionedBufferHandle buffer,
                   ResourceState state) override;

    VersionedTextureHandle writeTexture(RenderGraphPassHandle pass,
                                       const char* name,
                                       VersionedTextureHandle texture,
                                       ResourceState state) override;
    VersionedBufferHandle writeBuffer(RenderGraphPassHandle pass,
                                     const char* name,
                                     VersionedBufferHandle buffer,
                                     ResourceState state) override;

    VersionedTextureHandle createRenderTarget(const char* name,
                                             const RenderGraphResourceDesc& desc) override;

    void setOutput(VersionedTextureHandle texture) override;

    // 队列同步支持
    void queueSync(RenderGraphPassHandle pass,
                  QueueType srcQueue,
                  QueueType dstQueue) override;
    void executeNextAsync(QueueType queueType) override;

    // Subpass合并支持
    void beginSubpassGroup(const char* name) override;
    void endSubpassGroup() override;

private:
    RenderGraph* graph;

    // 下一个Pass是否异步执行
    bool nextPassAsync = false;
    QueueType nextPassQueue = QueueType::Graphics;

    // 名称到资源的映射
    std::unordered_map<std::string, VersionedTextureHandle> textureByName;
    std::unordered_map<std::string, VersionedBufferHandle> bufferByName;

    // 当前构建的资源版本计数
    std::unordered_map<Handle, u32> resourceVersions;

    VersionedTextureHandle getOrCreateVersionedTexture(const char* name,
                                                      VersionedTextureHandle baseHandle);
    VersionedBufferHandle getOrCreateVersionedBuffer(const char* name,
                                                    VersionedBufferHandle baseHandle);
};

/**
 * 渲染图编译器
 */
class RenderGraphCompiler {
public:
    explicit RenderGraphCompiler(RenderGraph* graph);

    RenderGraphCompileResult compile();

private:
    RenderGraph* graph;

    // 编译步骤
    bool validate();
    void cullUnusedPasses();
    void topologicalSort();
    void calculateResourceLifetimes();
    void optimizeBarriers();
    void scheduleAsyncCompute();
};

/**
 * 资源分配器
 * 负责瞬态资源的内存别名分配
 */
class ResourceAllocator {
public:
    explicit ResourceAllocator(IResourceManager* resourceManager);

    std::vector<ResourceAllocation> allocate(
        const std::vector<ResourceNode*>& resources);

private:
    IResourceManager* resourceManager;

    // 分配算法
    void greedyAllocate(std::vector<ResourceAllocation>& allocations,
                       const std::vector<ResourceNode*>& sortedResources);
    u64 calculateResourceSize(const ResourceNode* resource);
    u64 alignSize(u64 size, u64 alignment);
};

/**
 * 屏障管理器
 */
class BarrierManager {
public:
    explicit BarrierManager(RenderGraph* graph);

    std::vector<BarrierInfo> generateBarriers(
        const std::vector<RenderGraphPassHandle>& executionOrder);

    void batchBarriers(std::vector<BarrierInfo>& barriers);

private:
    RenderGraph* graph;

    // 状态追踪
    std::unordered_map<VersionedResourceHandle, ResourceState> currentStates;

    // 辅助函数
    PipelineStage inferSourceStage(ResourceState state);
    PipelineStage inferDestStage(ResourceState state);
    AccessFlags inferSourceAccess(ResourceState state);
    AccessFlags inferDestAccess(ResourceState state);
};

/**
 * 渲染图执行器
 */
class RenderGraphExecutor {
public:
    RenderGraphExecutor(IResourceManager* resourceManager,
                       IPipelineManager* pipelineManager);

    RenderGraphExecuteResult execute(
        const std::vector<RenderGraphPassHandle>& executionOrder,
        const std::vector<ResourceAllocation>& allocations,
        const std::vector<BarrierInfo>& barriers);

    RenderGraphExecuteResult execute(
        const std::vector<RenderGraphPassHandle>& executionOrder,
        const std::vector<ResourceAllocation>& allocations,
        const std::vector<BarrierInfo>& barriers,
        ICommandBuffer* commandBuffer);

private:
    IResourceManager* resourceManager;
    IPipelineManager* pipelineManager;

    // 执行上下文
    class RenderPassContext : public IRenderPassContext {
    public:
        RenderPassContext(RenderGraphPassHandle pass,
                         const char* name,
                         RenderPassType type,
                         ICommandBuffer* cmdBuffer,
                         IResourceManager* resourceManager,
                         IPipelineManager* pipelineManager);

        // IRenderPassContext实现
        RenderGraphPassHandle getCurrentPass() const override;
        const char* getPassName() const override;
        RenderPassType getPassType() const override;
        ICommandBuffer* getCommandBuffer() override;

        TextureHandle getTexture(VersionedTextureHandle handle) override;
        BufferHandle getBuffer(VersionedBufferHandle handle) override;
        ResourceViewHandle getTextureView(VersionedTextureHandle handle,
                                         const TextureViewDesc& desc) override;
        ResourceViewHandle getBufferView(VersionedBufferHandle handle,
                                        const BufferViewDesc& desc) override;

        void setPipeline(PipelineHandle pipeline) override;
        void setDescriptorSet(u32 setIndex, DescriptorSetHandle descriptorSet) override;
        void setPushConstants(ShaderStage stages, u32 offset, u32 size, const void* data) override;

        void setViewport(const Viewport& viewport) override;
        void setViewports(const std::vector<Viewport>& viewports) override;
        void setScissor(const Rect& scissor) override;
        void setScissors(const std::vector<Rect>& scissors) override;
        void setBlendConstants(const std::array<float, 4>& constants) override;
        void setStencilReference(u8 reference) override;
        void setDepthBounds(float minBounds, float maxBounds) override;

        void draw(u32 vertexCount, u32 instanceCount,
                 u32 firstVertex, u32 firstInstance) override;
        void drawIndexed(u32 indexCount, u32 instanceCount,
                        u32 firstIndex, i32 vertexOffset, u32 firstInstance) override;
        void drawIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) override;
        void drawIndexedIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) override;

        void dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) override;
        void dispatchIndirect(BufferHandle buffer, u64 offset) override;

        void traceRays(u32 width, u32 height, u32 depth) override;

        void resourceBarrier(TextureHandle texture, ResourceState newState,
                            const SubresourceRange& range) override;
        void resourceBarrier(BufferHandle buffer, ResourceState newState) override;

        void beginQuery(QueryPoolHandle pool, u32 query) override;
        void endQuery(QueryPoolHandle pool, u32 query) override;
        void writeTimestamp(QueryPoolHandle pool, u32 query) override;

        void beginDebugMarker(const char* name, const Color& color) override;
        void endDebugMarker() override;
        void insertDebugMarker(const char* name, const Color& color) override;

    private:
        RenderGraphPassHandle passHandle;
        const char* passName;
        RenderPassType passType;
        ICommandBuffer* commandBuffer;
        IResourceManager* resourceManager;
        IPipelineManager* pipelineManager;
    };
};

} // namespace render_graph
} // namespace render