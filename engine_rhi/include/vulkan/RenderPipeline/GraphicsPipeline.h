#pragma once
#ifndef ENGINE_VULKAN_GRAPHICS_PIPELINE_H
#define ENGINE_VULKAN_GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#include "vulkan/Resources/VulkanResourceHandles.h"

namespace engine::rhi::vulkan {

// 顶点输入状态
struct VertexInputBinding {
    uint32_t binding = 0;
    uint32_t stride = 0;
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
};

struct VertexInputAttribute {
    uint32_t location = 0;
    uint32_t binding = 0;
    VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
    uint32_t offset = 0;
};

struct VertexInputState {
    std::vector<VertexInputBinding> bindings;
    std::vector<VertexInputAttribute> attributes;
};

// 光栅化状态
struct RasterizerState {
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    float lineWidth = 1.0f;
    VkBool32 depthClampEnable = VK_FALSE;
    VkBool32 rasterizerDiscardEnable = VK_FALSE;
    VkBool32 depthBiasEnable = VK_FALSE;
    float depthBiasConstantFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
};

// 多重采样状态
struct MultisampleState {
    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 sampleShadingEnable = VK_FALSE;
    float minSampleShading = 1.0f;
    VkSampleMask* pSampleMask = nullptr;
    VkBool32 alphaToCoverageEnable = VK_FALSE;
    VkBool32 alphaToOneEnable = VK_FALSE;
};

// 深度模板状态
struct DepthStencilState {
    VkBool32 depthTestEnable = VK_TRUE;
    VkBool32 depthWriteEnable = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    VkBool32 depthBoundsTestEnable = VK_FALSE;
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
    VkBool32 stencilTestEnable = VK_FALSE;
    VkStencilOpState front = {};
    VkStencilOpState back = {};
};

// 颜色混合附件状态
struct ColorBlendAttachment {
    VkBool32 blendEnable = VK_FALSE;
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
};

struct ColorBlendState {
    VkBool32 logicOpEnable = VK_FALSE;
    VkLogicOp logicOp = VK_LOGIC_OP_COPY;
    std::vector<ColorBlendAttachment> attachments;
    float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

// 动态状态
struct DynamicState {
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
};

// 视口状态（如果用动态状态，这里只是占位）
struct ViewportState {
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
};

// 渲染目标格式 - Dynamic Rendering 必需
struct RenderingTargetFormats {
    std::vector<VkFormat> colorFormats;  // 每个RT的格式
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    VkFormat stencilFormat = VK_FORMAT_UNDEFINED;
};

// 图形管线描述 - 强制 Dynamic Rendering，无传统 RenderPass
struct GraphicsPipelineDesc {
    // Shader
    std::vector<VulkanShaderHandle> shaders;  // VS, FS, etc.
    
    // 固定功能状态
    VertexInputState vertexInput;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 primitiveRestartEnable = VK_FALSE;
    RasterizerState rasterizer;
    MultisampleState multisample;
    DepthStencilState depthStencil;
    ColorBlendState colorBlend;
    DynamicState dynamic;
    // ViewportState 移除 - 强制使用动态 viewport/scissor
    
    // Pipeline Layout
    VulkanPipelineLayoutHandle layout;
    
    // 渲染目标格式 - Dynamic Rendering 必需
    RenderingTargetFormats renderTargets;
    
    // 调试名
    const char* debugName = nullptr;
};

// 图形管线资源
struct GraphicsPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;  // 缓存引用
    GraphicsPipelineDesc desc;  // 保存desc用于比较/哈希
    
    [[nodiscard]] bool isValid() const { return pipeline != VK_NULL_HANDLE; }
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_GRAPHICS_PIPELINE_H
