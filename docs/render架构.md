# 抽象渲染库完整设计

## 一、总体架构概览

### 1.1 系统架构图
```
┌─────────────────────────────────────────────────────────────┐
│                   应用程序 (Application)                       │
├─────────────────────────────────────────────────────────────┤
│                 渲染图管理器 (RenderGraphManager)              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐       │
│  │ 编译器      │  │ 优化器      │  │ 执行器      │       │
│  └─────────────┘  └─────────────┘  └─────────────┘       │
├─────────────────────────────────────────────────────────────┤
│           资源管理器 (ResourceManager)  │ 管线管理器 (PipelineManager) │
├─────────────────────────────────────────────────────────────┤
│              渲染设备抽象层 (RenderDevice)                    │
│  ┌─────────────────┐  ┌─────────────────┐                  │
│  │ Vulkan实现      │  │ 其他API实现      │                  │
│  └─────────────────┘  └─────────────────┘                  │
└─────────────────────────────────────────────────────────────┘
```

## 二、核心接口设计

### 2.1 顶层接口 (IRenderSystem)
```cpp
/**
 * 渲染系统顶层接口
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
    
    // 子系统访问
    virtual IRenderGraphManager* getRenderGraphManager() = 0;
    virtual IResourceManager* getResourceManager() = 0;
    virtual IPipelineManager* getPipelineManager() = 0;
    virtual IShaderManager* getShaderManager() = 0;
    
    // 查询功能
    virtual RenderAPI getRenderAPI() const = 0;
    virtual const RenderDeviceInfo& getDeviceInfo() const = 0;
    
    // 调试与工具
    virtual IRenderDebug* getDebugInterface() = 0;
    virtual IRenderProfiler* getProfiler() = 0;
    
    // 多线程支持
    virtual ICommandQueue* getGraphicsQueue() = 0;
    virtual ICommandQueue* getComputeQueue() = 0;
    virtual ICommandQueue* getTransferQueue() = 0;
};
```

### 2.2 渲染图管理器接口 (IRenderGraphManager)
```cpp
/**
 * 渲染图管理器接口
 */
class IRenderGraphManager {
public:
    struct RenderGraphDesc {
        std::string name;
        RenderGraphFlags flags = RenderGraphFlags::None;
        uint32_t maxPasses = 128;
        uint32_t maxResources = 256;
    };
    
    struct RenderPassDesc {
        std::string name;
        RenderPassType type = RenderPassType::Graphics;
        std::vector<ResourceHandle> inputs;
        std::vector<ResourceHandle> outputs;
        std::vector<ResourceHandle> internalResources;
        RenderPassFlags flags = RenderPassFlags::None;
        
        // 回调函数
        std::function<void(IRenderPassContext&)> setupCallback;
        std::function<void(IRenderPassContext&)> executeCallback;
        std::function<void(IRenderPassContext&)> teardownCallback;
    };
    
    struct ResourceDesc {
        std::string name;
        ResourceType type = ResourceType::Texture2D;
        Format format = Format::RGBA8_UNORM;
        ResourceExtent extent = {1, 1, 1};
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        ResourceUsage usage = ResourceUsage::ShaderResource;
        MemoryType memoryType = MemoryType::Default;
        ClearValue clearValue = {};
        ResourceState initialState = ResourceState::Undefined;
    };
    
    virtual ~IRenderGraphManager() = default;
    
    // 渲染图生命周期
    virtual RenderGraphHandle createRenderGraph(const RenderGraphDesc& desc) = 0;
    virtual void destroyRenderGraph(RenderGraphHandle handle) = 0;
    
    // 资源声明
    virtual ResourceHandle declareResource(const ResourceDesc& desc) = 0;
    virtual ResourceHandle importResource(ResourceHandle externalResource, 
                                         const ResourceDesc& desc) = 0;
    virtual ResourceHandle getResourceByName(const std::string& name) const = 0;
    
    // 渲染通道管理
    virtual RenderPassHandle addRenderPass(const RenderPassDesc& desc) = 0;
    virtual void removeRenderPass(RenderPassHandle handle) = 0;
    
    // 依赖关系
    virtual void addDependency(RenderPassHandle src, RenderPassHandle dst,
                              const DependencyInfo& dependency) = 0;
    
    // 编译与执行
    virtual RenderGraphCompileResult compile(RenderGraphHandle graph) = 0;
    virtual RenderGraphExecuteResult execute(RenderGraphHandle graph) = 0;
    
    // 查询与调试
    virtual const RenderGraphInfo& getGraphInfo(RenderGraphHandle graph) const = 0;
    virtual void dumpGraph(RenderGraphHandle graph, const std::string& filename) = 0;
    virtual void visualizeGraph(RenderGraphHandle graph) = 0;
    
    // 异步操作
    virtual Future<RenderGraphCompileResult> compileAsync(RenderGraphHandle graph) = 0;
    virtual Future<RenderGraphExecuteResult> executeAsync(RenderGraphHandle graph) = 0;
    
    // 高级功能
    virtual void setResourceAlias(ResourceHandle resourceA, 
                                 ResourceHandle resourceB) = 0;
    virtual void setPassAsync(RenderPassHandle pass, bool async) = 0;
    
    // 缓存管理
    virtual void clearCompilationCache() = 0;
    virtual void warmupCache(RenderGraphHandle graph) = 0;
};
```

### 2.3 资源管理器接口 (IResourceManager)
```cpp
/**
 * 资源管理器接口
 */
class IResourceManager {
public:
    struct TextureCreateInfo {
        ResourceExtent extent = {1, 1, 1};
        Format format = Format::RGBA8_UNORM;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        TextureType type = TextureType::Texture2D;
        ResourceUsage usage = ResourceUsage::ShaderResource;
        MemoryType memoryType = MemoryType::Default;
        ClearValue clearValue = {};
        TextureFlags flags = TextureFlags::None;
        std::string debugName;
    };
    
    struct BufferCreateInfo {
        uint64_t size = 0;
        BufferType type = BufferType::Default;
        ResourceUsage usage = ResourceUsage::VertexBuffer;
        MemoryType memoryType = MemoryType::Upload;
        std::optional<uint32_t> stride; // 结构化缓冲区
        std::string debugName;
    };
    
    struct ResourceViewDesc {
        ResourceViewType type = ResourceViewType::ShaderResource;
        Format format = Format::Unknown;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerCount = 1;
        ComponentMapping components = {};
    };
    
    virtual ~IResourceManager() = default;
    
    // 纹理管理
    virtual TextureHandle createTexture(const TextureCreateInfo& createInfo) = 0;
    virtual TextureHandle createTextureFromData(const void* data, 
                                               const TextureCreateInfo& createInfo,
                                               uint32_t pitch = 0) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual TextureInfo getTextureInfo(TextureHandle handle) const = 0;
    
    // 缓冲区管理
    virtual BufferHandle createBuffer(const BufferCreateInfo& createInfo) = 0;
    virtual BufferHandle createBufferFromData(const void* data, uint64_t size,
                                            const BufferCreateInfo& createInfo) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual BufferInfo getBufferInfo(BufferHandle handle) const = 0;
    
    // 资源视图
    virtual ResourceViewHandle createResourceView(ResourceHandle resource,
                                                 const ResourceViewDesc& desc) = 0;
    virtual void destroyResourceView(ResourceViewHandle handle) = 0;
    
    // 内存管理
    virtual void* mapResource(ResourceHandle handle, 
                             MapType mapType = MapType::ReadWrite) = 0;
    virtual void unmapResource(ResourceHandle handle) = 0;
    
    // 更新与复制
    virtual void updateTexture(TextureHandle texture, const TextureSubresourceData& data) = 0;
    virtual void updateBuffer(BufferHandle buffer, const void* data, 
                            uint64_t offset, uint64_t size) = 0;
    virtual void copyResource(ResourceHandle dst, ResourceHandle src) = 0;
    
    // 资源状态管理
    virtual void transitionResource(ResourceHandle handle, 
                                   ResourceState newState,
                                   uint32_t mipLevel = 0,
                                   uint32_t arrayLayer = 0) = 0;
    
    // 查询功能
    virtual uint64_t getResourceSize(ResourceHandle handle) const = 0;
    virtual MemoryType getResourceMemoryType(ResourceHandle handle) const = 0;
    virtual bool isResourceValid(ResourceHandle handle) const = 0;
    
    // 异步操作
    virtual Future<TextureHandle> createTextureAsync(const TextureCreateInfo& createInfo) = 0;
    virtual Future<void> uploadTextureAsync(TextureHandle texture, 
                                          const void* data) = 0;
    
    // 内存统计
    virtual MemoryStatistics getMemoryStatistics() const = 0;
    virtual void dumpMemoryInfo(const std::string& filename) const = 0;
    
    // 缓存管理
    virtual void setResourceCacheSize(uint64_t maxSize) = 0;
    virtual void clearResourceCache() = 0;
};
```

### 2.4 渲染设备抽象 (IRenderDevice)
```cpp
/**
 * 渲染设备抽象接口
 */
class IRenderDevice {
public:
    struct DeviceCreateInfo {
        RenderAPI api = RenderAPI::Vulkan;
        AdapterType adapterType = AdapterType::Discrete;
        FeatureLevel featureLevel = FeatureLevel::Level_1_0;
        DeviceFlags flags = DeviceFlags::None;
        
        // 窗口相关
        void* windowHandle = nullptr;
        Extent2D windowExtent = {800, 600};
        
        // 队列创建
        struct QueueCreateInfo {
            QueueType type = QueueType::Graphics;
            uint32_t count = 1;
            uint32_t priority = 1;
        };
        std::vector<QueueCreateInfo> queueCreateInfos;
        
        // 功能启用
        std::vector<std::string> requiredExtensions;
        std::vector<std::string> optionalExtensions;
        
        // 调试功能
        bool enableValidation = true;
        bool enableDebugMarkers = true;
        bool enableApiDump = false;
        
        // 多GPU支持
        uint32_t adapterIndex = 0;
    };
    
    virtual ~IRenderDevice() = default;
    
    // 设备管理
    virtual bool initialize(const DeviceCreateInfo& createInfo) = 0;
    virtual void shutdown() = 0;
    
    // 交换链管理
    virtual ISwapChain* createSwapChain(const SwapChainCreateInfo& createInfo) = 0;
    virtual void destroySwapChain(ISwapChain* swapChain) = 0;
    
    // 命令队列
    virtual ICommandQueue* getCommandQueue(QueueType type, uint32_t index = 0) = 0;
    virtual ICommandAllocator* createCommandAllocator(QueueType queueType) = 0;
    virtual void destroyCommandAllocator(ICommandAllocator* allocator) = 0;
    
    // 命令缓冲区
    virtual ICommandBuffer* createCommandBuffer(ICommandAllocator* allocator,
                                               CommandBufferLevel level) = 0;
    virtual void destroyCommandBuffer(ICommandBuffer* commandBuffer) = 0;
    
    // 同步对象
    virtual IFence* createFence(uint64_t initialValue = 0) = 0;
    virtual void destroyFence(IFence* fence) = 0;
    
    virtual ISemaphore* createSemaphore(uint64_t initialValue = 0) = 0;
    virtual void destroySemaphore(ISemaphore* semaphore) = 0;
    
    // 管线状态对象
    virtual IPipelineState* createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    virtual IPipelineState* createComputePipeline(const ComputePipelineDesc& desc) = 0;
    virtual void destroyPipelineState(IPipelineState* pipeline) = 0;
    
    // 描述符管理
    virtual IDescriptorPool* createDescriptorPool(const DescriptorPoolDesc& desc) = 0;
    virtual void destroyDescriptorPool(IDescriptorPool* pool) = 0;
    
    virtual IDescriptorSetLayout* createDescriptorSetLayout(
        const DescriptorSetLayoutDesc& desc) = 0;
    virtual void destroyDescriptorSetLayout(IDescriptorSetLayout* layout) = 0;
    
    // 查询功能
    virtual const DeviceInfo& getDeviceInfo() const = 0;
    virtual const AdapterInfo& getAdapterInfo() const = 0;
    virtual bool isFeatureSupported(Feature feature) const = 0;
    
    // 内存分配
    virtual IHeap* createHeap(const HeapDesc& desc) = 0;
    virtual void destroyHeap(IHeap* heap) = 0;
    
    // 调试工具
    virtual void beginDebugRegion(const std::string& name, const Color& color) = 0;
    virtual void endDebugRegion() = 0;
    virtual void setObjectName(uint64_t object, ObjectType type, 
                              const std::string& name) = 0;
    
    // 帧管理
    virtual void waitIdle() = 0;
    virtual void flush() = 0;
};
```

### 2.5 管线管理器接口 (IPipelineManager)
```cpp
/**
 * 管线管理器接口
 */
class IPipelineManager {
public:
    struct GraphicsPipelineDesc {
        // 着色器
        ShaderHandle vertexShader;
        ShaderHandle pixelShader;
        ShaderHandle domainShader = InvalidHandle;
        ShaderHandle hullShader = InvalidHandle;
        ShaderHandle geometryShader = InvalidHandle;
        
        // 顶点输入
        VertexInputState vertexInput = {};
        
        // 图元拓扑
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        
        // 光栅化状态
        RasterizerState rasterizerState = {};
        
        // 深度模板状态
        DepthStencilState depthStencilState = {};
        
        // 混合状态
        std::vector<BlendState> blendStates = {BlendState{}};
        BlendFactor srcBlendFactor = BlendFactor::One;
        BlendFactor dstBlendFactor = BlendFactor::Zero;
        
        // 视口和裁剪
        std::vector<Viewport> viewports = {{0, 0, 800, 600, 0, 1}};
        std::vector<Rect> scissors = {{{0, 0}, {800, 600}}};
        
        // 渲染目标
        std::vector<Format> renderTargetFormats = {Format::RGBA8_UNORM};
        Format depthStencilFormat = Format::Unknown;
        uint32_t sampleCount = 1;
        
        // 管线布局
        PipelineLayoutHandle pipelineLayout = InvalidHandle;
    };
    
    struct ComputePipelineDesc {
        ShaderHandle computeShader;
        PipelineLayoutHandle pipelineLayout = InvalidHandle;
    };
    
    struct RayTracingPipelineDesc {
        // 射线追踪管线配置
        std::vector<RayTracingShaderGroup> shaderGroups;
        uint32_t maxRecursionDepth = 1;
        PipelineLayoutHandle pipelineLayout = InvalidHandle;
    };
    
    virtual ~IPipelineManager() = default;
    
    // 管线创建
    virtual PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc,
                                                 const std::string& name = "") = 0;
    virtual PipelineHandle createComputePipeline(const ComputePipelineDesc& desc,
                                                const std::string& name = "") = 0;
    virtual PipelineHandle createRayTracingPipeline(const RayTracingPipelineDesc& desc,
                                                   const std::string& name = "") = 0;
    virtual void destroyPipeline(PipelineHandle handle) = 0;
    
    // 管线布局
    virtual PipelineLayoutHandle createPipelineLayout(
        const PipelineLayoutDesc& desc, const std::string& name = "") = 0;
    virtual void destroyPipelineLayout(PipelineLayoutHandle handle) = 0;
    
    // 着色器管理
    virtual ShaderHandle createShader(const ShaderCreateInfo& createInfo,
                                     const std::string& name = "") = 0;
    virtual void destroyShader(ShaderHandle handle) = 0;
    
    // 热重载
    virtual bool reloadShader(ShaderHandle handle, const void* data, size_t size) = 0;
    virtual bool reloadPipeline(PipelineHandle handle) = 0;
    
    // 查询功能
    virtual const PipelineInfo& getPipelineInfo(PipelineHandle handle) const = 0;
    virtual const ShaderInfo& getShaderInfo(ShaderHandle handle) const = 0;
    virtual PipelineHandle getPipelineByName(const std::string& name) const = 0;
    
    // 缓存管理
    virtual void setPipelineCacheSize(size_t maxSize) = 0;
    virtual void clearPipelineCache() = 0;
    virtual bool loadPipelineCache(const void* data, size_t size) = 0;
    virtual std::vector<uint8_t> savePipelineCache() = 0;
    
    // 异步编译
    virtual Future<PipelineHandle> createPipelineAsync(const GraphicsPipelineDesc& desc) = 0;
    virtual Future<void> waitForPipelineCompilation() = 0;
    
    // 调试功能
    virtual void dumpPipelineInfo(PipelineHandle handle, 
                                 const std::string& filename) = 0;
};
```

## 三、数据结构定义

### 3.1 基础类型
```cpp
namespace Render {
    // 句柄类型
    using Handle = uint32_t;
    static constexpr Handle InvalidHandle = 0;
    
    template<typename T>
    class HandleType {
    public:
        HandleType() : handle(InvalidHandle) {}
        explicit HandleType(Handle h) : handle(h) {}
        
        bool isValid() const { return handle != InvalidHandle; }
        Handle get() const { return handle; }
        operator Handle() const { return handle; }
        
    private:
        Handle handle;
    };
    
    // 资源句柄
    using ResourceHandle = HandleType<struct ResourceTag>;
    using TextureHandle = HandleType<struct TextureTag>;
    using BufferHandle = HandleType<struct BufferTag>;
    using PipelineHandle = HandleType<struct PipelineTag>;
    using ShaderHandle = HandleType<struct ShaderTag>;
    using RenderGraphHandle = HandleType<struct RenderGraphTag>;
    using RenderPassHandle = HandleType<struct RenderPassTag>;
    
    // 枚举类型
    enum class RenderAPI {
        Vulkan,
        DirectX12,
        Metal,
        OpenGL
    };
    
    enum class ResourceType {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
        Buffer
    };
    
    enum class ResourceState {
        Undefined,
        Common,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        RenderTarget,
        DepthWrite,
        DepthRead,
        ShaderResource,
        UnorderedAccess,
        CopySource,
        CopyDest,
        Present
    };
    
    enum class Format {
        Unknown,
        RGBA8_UNORM,
        RGBA8_SRGB,
        RGBA16_FLOAT,
        RG32_FLOAT,
        D24_UNORM_S8_UINT,
        D32_FLOAT,
        // ... 更多格式
    };
    
    // 结构体定义
    struct ResourceExtent {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        
        ResourceExtent(uint32_t w = 1, uint32_t h = 1, uint32_t d = 1)
            : width(w), height(h), depth(d) {}
    };
    
    struct ClearValue {
        union {
            float color[4];
            struct {
                float depth;
                uint8_t stencil;
            };
        };
        
        ClearValue() {
            memset(color, 0, sizeof(color));
        }
        
        ClearValue(float r, float g, float b, float a) {
            color[0] = r; color[1] = g; color[2] = b; color[3] = a;
        }
    };
    
    struct Viewport {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };
    
    struct Rect {
        int32_t x = 0;
        int32_t y = 0;
        uint32_t width = 0;
        uint32_t height = 0;
    };
    
    // 描述符
    struct DescriptorSetLayoutBinding {
        uint32_t binding = 0;
        DescriptorType type = DescriptorType::Sampler;
        uint32_t count = 1;
        ShaderStageFlags stageFlags = ShaderStageFlags::All;
        SamplerHandle immutableSampler = InvalidHandle;
    };
    
    struct PipelineLayoutDesc {
        std::vector<DescriptorSetLayoutBinding> bindings;
        std::vector<PushConstantRange> pushConstants;
        PipelineLayoutFlags flags = PipelineLayoutFlags::None;
    };
    
    // 顶点输入
    struct VertexInputAttribute {
        uint32_t location = 0;
        uint32_t binding = 0;
        Format format = Format::RGBA32_FLOAT;
        uint32_t offset = 0;
    };
    
    struct VertexInputBinding {
        uint32_t binding = 0;
        uint32_t stride = 0;
        VertexInputRate inputRate = VertexInputRate::PerVertex;
    };
    
    struct VertexInputState {
        std::vector<VertexInputBinding> bindings;
        std::vector<VertexInputAttribute> attributes;
    };
}
```

### 3.2 渲染图相关数据结构
```cpp
namespace RenderGraph {
    // 渲染图节点
    struct RenderPassNode {
        std::string name;
        RenderPassHandle handle = InvalidHandle;
        RenderPassType type = RenderPassType::Graphics;
        
        // 依赖关系
        std::vector<RenderPassHandle> predecessors;
        std::vector<RenderPassHandle> successors;
        
        // 资源
        std::vector<ResourceHandle> inputs;
        std::vector<ResourceHandle> outputs;
        std::vector<ResourceHandle> internalResources;
        
        // 执行信息
        std::function<void(IRenderPassContext&)> setupCallback;
        std::function<void(IRenderPassContext&)> executeCallback;
        std::function<void(IRenderPassContext&)> teardownCallback;
        
        // 状态
        RenderPassFlags flags = RenderPassFlags::None;
        uint32_t executionOrder = 0;
        bool compiled = false;
        bool async = false;
    };
    
    // 资源节点
    struct ResourceNode {
        std::string name;
        ResourceHandle handle = InvalidHandle;
        ResourceType type = ResourceType::Texture2D;
        
        // 资源描述
        ResourceDesc desc;
        
        // 生命周期
        uint32_t firstUsePass = 0;
        uint32_t lastUsePass = 0;
        
        // 别名信息
        ResourceHandle aliasedResource = InvalidHandle;
        bool isAlias = false;
        
        // 状态跟踪
        ResourceState currentState = ResourceState::Undefined;
        std::vector<std::pair<uint32_t, ResourceState>> stateHistory;
        
        // 实际资源
        ResourceHandle actualResource = InvalidHandle;
    };
    
    // 边（依赖关系）
    struct DependencyEdge {
        RenderPassHandle fromPass = InvalidHandle;
        RenderPassHandle toPass = InvalidHandle;
        ResourceHandle resource = InvalidHandle;
        DependencyType type = DependencyType::ReadAfterWrite;
        
        // 同步信息
        PipelineStageFlags srcStageMask = PipelineStageFlags::AllCommands;
        PipelineStageFlags dstStageMask = PipelineStageFlags::AllCommands;
        AccessFlags srcAccessMask = AccessFlags::None;
        AccessFlags dstAccessMask = AccessFlags::None;
    };
    
    // 编译结果
    struct CompilationResult {
        bool success = false;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        // 执行顺序
        std::vector<RenderPassHandle> executionOrder;
        
        // 资源分配
        std::vector<ResourceAllocation> resourceAllocations;
        
        // 内存需求
        uint64_t totalMemory = 0;
        uint64_t peakMemory = 0;
        
        // 性能估计
        uint32_t estimatedCycles = 0;
        std::vector<RenderPassTiming> passTimings;
    };
    
    // 执行上下文
    class RenderPassContext : public IRenderPassContext {
    public:
        RenderPassContext(RenderPassHandle pass, ICommandBuffer* cmdBuffer)
            : passHandle(pass), commandBuffer(cmdBuffer) {}
        
        // 资源访问
        virtual TextureViewHandle getTextureView(ResourceHandle resource,
                                               const TextureViewDesc& desc = {}) override {
            return graph->getResourceTextureView(resource, desc);
        }
        
        virtual BufferViewHandle getBufferView(ResourceHandle resource,
                                             const BufferViewDesc& desc = {}) override {
            return graph->getResourceBufferView(resource, desc);
        }
        
        // 管线绑定
        virtual void setPipeline(PipelineHandle pipeline) override {
            commandBuffer->bindPipeline(pipeline);
            currentPipeline = pipeline;
        }
        
        virtual void setDescriptorSet(uint32_t setIndex, 
                                     DescriptorSetHandle descriptorSet) override {
            commandBuffer->bindDescriptorSet(setIndex, descriptorSet);
        }
        
        // 绘图命令
        virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                         uint32_t firstVertex = 0, uint32_t firstInstance = 0) override {
            commandBuffer->draw(vertexCount, instanceCount, firstVertex, firstInstance);
        }
        
        virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                                uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                                uint32_t firstInstance = 0) override {
            commandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, 
                                      vertexOffset, firstInstance);
        }
        
        virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY = 1,
                            uint32_t groupCountZ = 1) override {
            commandBuffer->dispatch(groupCountX, groupCountY, groupCountZ);
        }
        
        // 资源屏障
        virtual void resourceBarrier(const ResourceBarrier& barrier) override {
            commandBuffer->resourceBarrier(barrier);
        }
        
        // 查询功能
        virtual RenderPassHandle getCurrentPass() const override { return passHandle; }
        virtual ICommandBuffer* getCommandBuffer() override { return commandBuffer; }
        
    private:
        RenderPassHandle passHandle;
        ICommandBuffer* commandBuffer;
        PipelineHandle currentPipeline = InvalidHandle;
        RenderGraph* graph = nullptr;
    };
}
```

## 四、Vulkan后端实现示例

### 4.1 Vulkan设备实现
```cpp
class VulkanDevice : public IRenderDevice {
public:
    bool initialize(const DeviceCreateInfo& createInfo) override {
        // 1. 创建Vulkan实例
        vk::ApplicationInfo appInfo = {};
        appInfo.pApplicationName = createInfo.appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "RenderGraph";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
        
        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef VK_USE_PLATFORM_WIN32_KHR
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #endif
        };
        
        if (createInfo.enableValidation) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        
        vk::InstanceCreateInfo instanceInfo = {};
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensions.data();
        
        // 验证层
        std::vector<const char*> validationLayers;
        if (createInfo.enableValidation) {
            validationLayers = {"VK_LAYER_KHRONOS_validation"};
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            instanceInfo.ppEnabledLayerNames = validationLayers.data();
        }
        
        try {
            instance = vk::createInstance(instanceInfo);
        } catch (const vk::SystemError& e) {
            LOG_ERROR("Failed to create Vulkan instance: {}", e.what());
            return false;
        }
        
        // 2. 选择物理设备
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.empty()) {
            LOG_ERROR("No Vulkan physical devices found");
            return false;
        }
        
        // 选择适配器
        physicalDevice = selectPhysicalDevice(physicalDevices, createInfo.adapterType);
        
        // 3. 创建设备
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.queueFamilyIndex = graphicsQueueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
        };
        
        vk::PhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fillModeNonSolid = VK_TRUE;
        
        vk::DeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        
        if (createInfo.enableValidation) {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        }
        
        device = physicalDevice.createDevice(deviceCreateInfo);
        
        // 4. 获取队列
        graphicsQueue = device.getQueue(graphicsQueueFamily, 0);
        
        // 5. 创建命令池
        vk::CommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.queueFamilyIndex = graphicsQueueFamily;
        cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        commandPool = device.createCommandPool(cmdPoolInfo);
        
        // 6. 创建描述符池
        std::array<vk::DescriptorPoolSize, 4> poolSizes = {{
            {vk::DescriptorType::eSampler, 1000},
            {vk::DescriptorType::eCombinedImageSampler, 1000},
            {vk::DescriptorType::eSampledImage, 1000},
            {vk::DescriptorType::eStorageImage, 1000}
        }};
        
        vk::DescriptorPoolCreateInfo descPoolInfo = {};
        descPoolInfo.maxSets = 1000;
        descPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descPoolInfo.pPoolSizes = poolSizes.data();
        
        descriptorPool = device.createDescriptorPool(descPoolInfo);
        
        LOG_INFO("Vulkan device initialized successfully");
        return true;
    }
    
private:
    vk::PhysicalDevice selectPhysicalDevice(const std::vector<vk::PhysicalDevice>& devices,
                                           AdapterType preferredType) {
        struct DeviceScore {
            vk::PhysicalDevice device;
            uint32_t score = 0;
        };
        
        std::vector<DeviceScore> scoredDevices;
        
        for (const auto& device : devices) {
            DeviceScore score;
            score.device = device;
            
            auto properties = device.getProperties();
            auto features = device.getFeatures();
            
            // 基础分数
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                score.score += 1000;
            } else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
                score.score += 500;
            }
            
            // 检查队列家族
            auto queueFamilies = device.getQueueFamilyProperties();
            bool hasGraphicsQueue = false;
            for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    score.graphicsQueueFamily = i;
                    hasGraphicsQueue = true;
                    break;
                }
            }
            
            if (!hasGraphicsQueue) {
                continue; // 跳过没有图形队列的设备
            }
            
            // 检查扩展支持
            auto extensions = device.enumerateDeviceExtensionProperties();
            bool hasSwapchain = false;
            for (const auto& extension : extensions) {
                if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    hasSwapchain = true;
                    break;
                }
            }
            
            if (!hasSwapchain) {
                continue; // 跳过不支持交换链的设备
            }
            
            // 内存大小
            auto memoryProperties = device.getMemoryProperties();
            uint64_t deviceMemory = 0;
            for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i) {
                if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                    deviceMemory += memoryProperties.memoryHeaps[i].size;
                }
            }
            
            score.score += static_cast<uint32_t>(deviceMemory / (1024 * 1024)); // 每MB加1分
            
            scoredDevices.push_back(score);
        }
        
        if (scoredDevices.empty()) {
            throw std::runtime_error("No suitable Vulkan device found");
        }
        
        // 选择分数最高的设备
        std::sort(scoredDevices.begin(), scoredDevices.end(),
                 const DeviceScore& a, const DeviceScore& b {
                     return a.score > b.score;
                 });
        
        graphicsQueueFamily = scoredDevices[0].graphicsQueueFamily;
        return scoredDevices[0].device;
    }
    
private:
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::Queue graphicsQueue;
    vk::CommandPool commandPool;
    vk::DescriptorPool descriptorPool;
    uint32_t graphicsQueueFamily = 0;
};
```

### 4.2 Vulkan资源实现
```cpp
class VulkanResourceManager : public IResourceManager {
public:
    TextureHandle createTexture(const TextureCreateInfo& createInfo) override {
        VulkanTexture texture;
        
        // 创建Vulkan图像
        vk::ImageCreateInfo imageInfo = {};
        imageInfo.imageType = getVulkanImageType(createInfo.type);
        imageInfo.format = convertFormat(createInfo.format);
        imageInfo.extent = vk::Extent3D{
            createInfo.extent.width,
            createInfo.extent.height,
            createInfo.extent.depth
        };
        imageInfo.mipLevels = createInfo.mipLevels;
        imageInfo.arrayLayers = createInfo.arrayLayers;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.usage = convertUsage(createInfo.usage);
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        
        texture.image = device.createImage(imageInfo);
        
        // 获取内存需求
        auto memRequirements = device.getImageMemoryRequirements(texture.image);
        
        // 分配内存
        vk::MemoryAllocateInfo allocInfo = {};
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                                   convertMemoryType(createInfo.memoryType));
        
        texture.memory = device.allocateMemory(allocInfo);
        device.bindImageMemory(texture.image, texture.memory, 0);
        
        // 创建图像视图
        vk::ImageViewCreateInfo viewInfo = {};
        viewInfo.image = texture.image;
        viewInfo.viewType = getImageViewType(createInfo.type);
        viewInfo.format = convertFormat(createInfo.format);
        viewInfo.subresourceRange.aspectMask = getAspectMask(createInfo.format);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = createInfo.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = createInfo.arrayLayers;
        
        texture.view = device.createImageView(viewInfo);
        
        // 记录纹理信息
        texture.info = createInfo;
        texture.currentLayout = vk::ImageLayout::eUndefined;
        
        // 设置调试名称
        if (!createInfo.debugName.empty()) {
            setDebugName(texture.image, createInfo.debugName + "_Image");
            setDebugName(texture.view, createInfo.debugName + "_View");
        }
        
        // 存储纹理
        TextureHandle handle = textureHandles.allocate();
        textures[handle] = std::move(texture);
        
        return handle;
    }
    
    void* mapResource(ResourceHandle handle, MapType mapType) override {
        auto it = resources.find(handle);
        if (it == resources.end()) {
            return nullptr;
        }
        
        VulkanResource& resource = it->second;
        
        if (!resource.isMappable) {
            LOG_ERROR("Resource is not mappable");
            return nullptr;
        }
        
        if (resource.mappedPtr) {
            return resource.mappedPtr; // 已经映射
        }
        
        resource.mappedPtr = device.mapMemory(resource.memory, 0, VK_WHOLE_SIZE, {});
        return resource.mappedPtr;
    }
    
private:
    struct VulkanTexture {
        vk::Image image;
        vk::DeviceMemory memory;
        vk::ImageView view;
        vk::ImageLayout currentLayout;
        TextureCreateInfo info;
        std::string debugName;
    };
    
    struct VulkanBuffer {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        BufferCreateInfo info;
        void* mappedPtr = nullptr;
        bool isMappable = false;
    };
    
    HandleAllocator<TextureHandle> textureHandles;
    HandleAllocator<BufferHandle> bufferHandles;
    std::unordered_map<TextureHandle, VulkanTexture> textures;
    std::unordered_map<BufferHandle, VulkanBuffer> buffers;
    vk::Device device;
    
    // 辅助函数
    vk::ImageType getVulkanImageType(TextureType type) {
        switch (type) {
            case TextureType::Texture1D: return vk::ImageType::e1D;
            case TextureType::Texture2D: return vk::ImageType::e2D;
            case TextureType::Texture3D: return vk::ImageType::e3D;
            case TextureType::TextureCube: return vk::ImageType::e2D;
            default: return vk::ImageType::e2D;
        }
    }
    
    vk::ImageViewType getImageViewType(TextureType type) {
        switch (type) {
            case TextureType::Texture1D: return vk::ImageViewType::e1D;
            case TextureType::Texture2D: return vk::ImageViewType::e2D;
            case TextureType::Texture3D: return vk::ImageViewType::e3D;
            case TextureType::TextureCube: return vk::ImageViewType::eCube;
            default: return vk::ImageViewType::e2D;
        }
    }
};
```

## 五、高级特性设计

### 5.1 异步编译与执行
```cpp
class AsyncRenderGraphCompiler {
public:
    struct CompileTask {
        RenderGraphHandle graph;
        std::promise<CompilationResult> promise;
        std::atomic<bool> cancelled{false};
    };
    
    AsyncRenderGraphCompiler(uint32_t numThreads = std::thread::hardware_concurrency()) {
        for (uint32_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] { workerThread(); });
        }
    }
    
    ~AsyncRenderGraphCompiler() {
        stop = true;
        cv.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }
    
    std::future<CompilationResult> compileAsync(RenderGraphHandle graph) {
        CompileTask task;
        task.graph = graph;
        auto future = task.promise.get_future();
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            compileQueue.push(std::move(task));
        }
        
        cv.notify_one();
        return future;
    }
    
    void cancelAll() {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!compileQueue.empty()) {
            compileQueue.front().cancelled = true;
            compileQueue.front().promise.set_value({});
            compileQueue.pop();
        }
    }
    
private:
    void workerThread() {
        while (!stop) {
            CompileTask task;
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] { return stop || !compileQueue.empty(); });
                
                if (stop) return;
                
                task = std::move(compileQueue.front());
                compileQueue.pop();
            }
            
            if (task.cancelled) continue;
            
            try {
                CompilationResult result = compileGraph(task.graph);
                task.promise.set_value(std::move(result));
            } catch (const std::exception& e) {
                CompilationResult result;
                result.success = false;
                result.errors.push_back(e.what());
                task.promise.set_value(std::move(result));
            }
        }
    }
    
private:
    std::vector<std::thread> workers;
    std::queue<CompileTask> compileQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> stop{false};
};
```

### 5.2 资源别名优化
```cpp
class ResourceAliasAllocator {
public:
    struct Allocation {
        ResourceHandle resource;
        uint64_t offset = 0;
        uint64_t size = 0;
        uint32_t firstUse = 0;
        uint32_t lastUse = 0;
    };
    
    std::vector<Allocation> allocateWithAliasing(const std::vector<ResourceNode>& resources) {
        // 1. 按首次使用时间排序
        std::vector<const ResourceNode*> sortedResources;
        for (const auto& resource : resources) {
            sortedResources.push_back(&resource);
        }
        
        std::sort(sortedResources.begin(), sortedResources.end(),
                 const ResourceNode* a, const ResourceNode* b {
                     return a->firstUsePass < b->firstUsePass;
                 });
        
        // 2. 贪心算法分配内存
        std::vector<Allocation> allocations;
        std::vector<Allocation> activeAllocations;
        
        for (const auto* resource : sortedResources) {
            // 移除已结束使用的分配
            activeAllocations.erase(
                std::remove_if(activeAllocations.begin(), activeAllocations.end(),
                             const Allocation& alloc {
                                 return alloc.lastUse < resource->firstUsePass;
                             }),
                activeAllocations.end()
            );
            
            // 寻找空闲空间
            bool allocated = false;
            for (auto& allocation : activeAllocations) {
                if (allocation.resource.get() == InvalidHandle) { // 空闲槽位
                    allocation.resource = resource->handle;
                    allocation.size = calculateSize(*resource);
                    allocation.firstUse = resource->firstUsePass;
                    allocation.lastUse = resource->lastUsePass;
                    allocated = true;
                    break;
                }
            }
            
            if (!allocated) {
                // 创建新分配
                Allocation allocation;
                allocation.resource = resource->handle;
                allocation.offset = totalSize;
                allocation.size = calculateSize(*resource);
                allocation.firstUse = resource->firstUsePass;
                allocation.lastUse = resource->lastUsePass;
                
                allocations.push_back(allocation);
                activeAllocations.push_back(allocation);
                totalSize += allocation.size;
            }
        }
        
        return allocations;
    }
    
private:
    uint64_t totalSize = 0;
};
```

## 六、使用示例

### 6.1 创建渲染系统
```cpp
// 1. 创建渲染系统
RenderSystemConfig config;
config.api = RenderAPI::Vulkan;
config.enableValidation = true;
config.windowHandle = window->getNativeHandle();
config.windowExtent = {1280, 720};

std::unique_ptr<IRenderSystem> renderSystem = CreateRenderSystem();
if (!renderSystem->initialize(config)) {
    LOG_ERROR("Failed to initialize render system");
    return;
}

// 2. 获取管理器
auto* renderGraphManager = renderSystem->getRenderGraphManager();
auto* resourceManager = renderSystem->getResourceManager();
auto* pipelineManager = renderSystem->getPipelineManager();

// 3. 创建渲染图
RenderGraphDesc graphDesc;
graphDesc.name = "MainRenderGraph";
graphDesc.maxPasses = 16;
graphDesc.maxResources = 32;

RenderGraphHandle renderGraph = renderGraphManager->createRenderGraph(graphDesc);

// 4. 声明资源
ResourceDesc depthDesc;
depthDesc.name = "DepthBuffer";
depthDesc.type = ResourceType::Texture2D;
depthDesc.format = Format::D24_UNORM_S8_UINT;
depthDesc.extent = {1280, 720, 1};
depthDesc.usage = ResourceUsage::DepthStencil;
ResourceHandle depthBuffer = renderGraphManager->declareResource(depthDesc);

ResourceDesc colorDesc;
colorDesc.name = "ColorBuffer";
colorDesc.type = ResourceType::Texture2D;
colorDesc.format = Format::RGBA8_UNORM;
colorDesc.extent = {1280, 720, 1};
colorDesc.usage = ResourceUsage::RenderTarget;
ResourceHandle colorBuffer = renderGraphManager->declareResource(colorDesc);

// 5. 添加渲染通道
RenderPassDesc gbufferPassDesc;
gbufferPassDesc.name = "GBufferPass";
gbufferPassDesc.type = RenderPassType::Graphics;
gbufferPassDesc.outputs = {depthBuffer, colorBuffer};
gbufferPassDesc.executeCallback = IRenderPassContext& context {
    // 设置管线
    context.setPipeline(gbufferPipeline);
    
    // 绑定描述符集
    context.setDescriptorSet(0, frameDescriptorSet);
    context.setDescriptorSet(1, materialDescriptorSet);
    
    // 绘制场景
    for (const auto& mesh : scene->getMeshes()) {
        context.setDescriptorSet(2, mesh.descriptorSet);
        context.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
    }
};

RenderPassHandle gbufferPass = renderGraphManager->addRenderPass(gbufferPassDesc);

// 6. 编译渲染图
auto compileResult = renderGraphManager->compile(renderGraph);
if (!compileResult.success) {
    for (const auto& error : compileResult.errors) {
        LOG_ERROR("Compilation error: {}", error);
    }
    return;
}

// 7. 主循环
while (!window->shouldClose()) {
    // 开始帧
    renderSystem->beginFrame(frameInfo);
    
    // 更新资源
    updateResources();
    
    // 执行渲染图
    renderGraphManager->execute(renderGraph);
    
    // 结束帧
    renderSystem->endFrame();
    
    // 呈现
    swapChain->present();
}
```

## 七、扩展性设计

### 7.1 插件系统
```cpp
class RenderPlugin {
public:
    virtual ~RenderPlugin() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    
    virtual bool initialize(IRenderSystem* renderSystem) = 0;
    virtual void shutdown() = 0;
    
    virtual void update(float deltaTime) = 0;
    virtual void render(IRenderPassContext& context) = 0;
    
    virtual std::vector<std::string> getRequiredExtensions() const = 0;
    virtual std::vector<Feature> getRequiredFeatures() const = 0;
};

class PluginManager {
public:
    void registerPlugin(std::unique_ptr<RenderPlugin> plugin) {
        plugins.push_back(std::move(plugin));
    }
    
    void initializeAll(IRenderSystem* renderSystem) {
        for (auto& plugin : plugins) {
            if (plugin->initialize(renderSystem)) {
                LOG_INFO("Plugin {} initialized", plugin->getName());
            }
        }
    }
    
private:
    std::vector<std::unique_ptr<RenderPlugin>> plugins;
};
```

### 7.2 多后端支持
```cpp
class RenderSystemFactory {
public:
    static std::unique_ptr<IRenderSystem> create(RenderAPI api) {
        switch (api) {
            case RenderAPI::Vulkan:
                return std::make_unique<VulkanRenderSystem>();
            case RenderAPI::DirectX12:
            #ifdef RENDER_SYSTEM_DX12
                return std::make_unique<D3D12RenderSystem>();
            #else
                LOG_ERROR("DirectX12 not supported");
                return nullptr;
            #endif
            case RenderAPI::Metal:
            #ifdef RENDER_SYSTEM_METAL
                return std::make_unique<MetalRenderSystem>();
            #else
                LOG_ERROR("Metal not supported");
                return nullptr;
            #endif
            default:
                LOG_ERROR("Unknown render API");
                return nullptr;
        }
    }
};
```

## 八、性能优化建议

1. **内存管理优化**：
    - 使用内存池减少分配开销
    - 实现资源别名减少内存占用
    - 使用子分配器管理大块内存

2. **多线程优化**：
    - 命令录制多线程化
    - 资源上传异步化
    - 管线编译预编译

3. **缓存优化**：
    - 着色器编译缓存
    - 管线状态对象缓存
    - 描述符集缓存

4. **渲染图优化**：
    - 自动Pass合并
    - 依赖关系简化
    - 屏障合并优化

这个设计提供了完整的抽象渲染库框架，支持rendergraph和Vulkan后端，具有良好的扩展性、性能优化空间和易用性。