#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class IRenderDevice;

/**
 * 着色器创建信息
 */
struct ShaderCreateInfo {
    ShaderStage stage = ShaderStage::Vertex;
    const void* code = nullptr;
    size_t codeSize = 0;
    std::string entryPoint = "main";
    std::string debugName;

    // 从文件加载
    static ShaderCreateInfo fromFile(ShaderStage stage, const char* filename,
                                    const char* entryPoint = "main");
    // 从内存加载
    static ShaderCreateInfo fromMemory(ShaderStage stage, const void* code, size_t size,
                                      const char* entryPoint = "main");
    // SPIR-V二进制
    static ShaderCreateInfo fromSpirv(ShaderStage stage, const void* spirv, size_t size,
                                     const char* entryPoint = "main");
};

/**
 * 着色器信息
 */
struct ShaderInfo {
    ShaderStage stage;
    std::string entryPoint;
    size_t codeSize;
    std::string debugName;
};

/**
 * 管线布局描述
 */
struct PipelineLayoutDesc {
    std::vector<DescriptorSetLayoutHandle> setLayouts;
    struct PushConstantRange {
        ShaderStage stages = ShaderStage::None;
        u32 offset = 0;
        u32 size = 0;
    };
    std::vector<PushConstantRange> pushConstants;
};

/**
 * 图形管线描述
 */
struct GraphicsPipelineDesc {
    // 着色器
    ShaderHandle vertexShader;
    ShaderHandle pixelShader;
    ShaderHandle domainShader;
    ShaderHandle hullShader;
    ShaderHandle geometryShader;

    // 顶点输入
    std::vector<VertexInputBinding> vertexBindings;
    std::vector<VertexInputAttribute> vertexAttributes;

    // 输入装配
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    bool primitiveRestartEnable = false;

    // 视口和裁剪
    std::vector<Viewport> viewports;
    std::vector<Rect> scissors;
    bool viewportDynamic = true;
    bool scissorDynamic = true;

    // 光栅化
    RasterizerState rasterizerState;

    // 多重采样
    u32 sampleCount = 1;
    u32 sampleMask = 0xFFFFFFFF;
    bool alphaToCoverageEnable = false;
    bool alphaToOneEnable = false;

    // 深度模板
    DepthStencilState depthStencilState;
    bool depthBoundsDynamic = false;
    bool stencilReferenceDynamic = false;

    // 混合
    BlendState blendState;
    std::array<float, 4> blendConstants = {0.0f, 0.0f, 0.0f, 0.0f};

    // 渲染目标
    std::vector<Format> renderTargetFormats;
    Format depthStencilFormat = Format::Unknown;

    // 管线布局
    PipelineLayoutHandle pipelineLayout;

    // 动态状态
    bool lineWidthDynamic = false;

    std::string debugName;
};

/**
 * 计算管线描述
 */
struct ComputePipelineDesc {
    ShaderHandle computeShader;
    PipelineLayoutHandle pipelineLayout;
    std::string debugName;
};

/**
 * 光追管线描述
 */
struct RayTracingPipelineDesc {
    struct ShaderGroup {
        /**
         * @brief 着色器组类型
         * @details 光线追踪管线中的着色器分组
         */
        enum class Type {
            General,        ///< 通用着色器（RayGen, Miss, Callable）
            TrianglesHit,   ///< 三角形命中组（ClosestHit + AnyHit）
            ProceduralHit   ///< 过程命中组（Intersection + ClosestHit + AnyHit）
        };
        Type type;
        ShaderHandle generalShader;      // For General
        ShaderHandle closestHitShader;   // For hit groups
        ShaderHandle anyHitShader;       // For hit groups
        ShaderHandle intersectionShader; // For procedural hit groups
        u32 groupIndex = 0;
    };

    std::vector<ShaderGroup> shaderGroups;
    u32 maxRecursionDepth = 1;
    PipelineLayoutHandle pipelineLayout;
    std::string debugName;
};

/**
 * 管线信息
 */
struct PipelineInfo {
    /**
     * @brief 管线类型
     * @details 区分不同类型的渲染管线
     */
    enum class Type {
        Graphics,   ///< 图形管线
        Compute,    ///< 计算管线
        RayTracing  ///< 光线追踪管线
    };
    Type type;
    PipelineLayoutHandle layout;
    std::string debugName;
    u64 hash; // 用于缓存
};

/**
 * 管线管理器接口
 * 负责管线状态对象（PSO）和着色器的管理
 */
class IPipelineManager {
public:
    virtual ~IPipelineManager() = default;

    // 着色器管理
    virtual ShaderHandle createShader(const ShaderCreateInfo& createInfo) = 0;
    virtual void destroyShader(ShaderHandle handle) = 0;
    virtual ShaderInfo getShaderInfo(ShaderHandle handle) const = 0;
    virtual ShaderHandle getShaderByName(const char* name) const = 0;

    // 管线布局
    virtual PipelineLayoutHandle createPipelineLayout(const PipelineLayoutDesc& desc) = 0;
    virtual void destroyPipelineLayout(PipelineLayoutHandle handle) = 0;

    // 图形管线
    virtual PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc) = 0;
    virtual PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc,
                                                 const char* name) = 0;

    // 计算管线
    virtual PipelineHandle createComputePipeline(const ComputePipelineDesc& desc) = 0;
    virtual PipelineHandle createComputePipeline(const ComputePipelineDesc& desc,
                                                const char* name) = 0;

    // 光追管线
    virtual PipelineHandle createRayTracingPipeline(const RayTracingPipelineDesc& desc) = 0;

    // 管线销毁
    virtual void destroyPipeline(PipelineHandle handle) = 0;

    // 查询
    virtual PipelineInfo getPipelineInfo(PipelineHandle handle) const = 0;
    virtual PipelineHandle getPipelineByName(const char* name) const = 0;

    // 热重载
    virtual bool reloadShader(ShaderHandle handle, const void* data, size_t size) = 0;
    virtual bool reloadPipeline(PipelineHandle handle) = 0;
    virtual void enableHotReload(bool enable) = 0;
    virtual void checkForChanges() = 0;

    // 缓存管理
    virtual void setPipelineCacheSize(size_t maxSize) = 0;
    virtual void clearPipelineCache() = 0;
    virtual bool loadPipelineCache(const void* data, size_t size) = 0;
    virtual std::vector<u8> savePipelineCache() = 0;

    // 异步编译
    virtual void compileAsync(PipelineHandle handle) = 0;
    virtual bool isCompilationComplete(PipelineHandle handle) = 0;
    virtual void waitForCompilation() = 0;

    // 调试功能
    virtual void dumpPipelineInfo(PipelineHandle handle, const char* filename) = 0;
    virtual void dumpAllPipelines(const char* filename) = 0;
};

} // namespace render