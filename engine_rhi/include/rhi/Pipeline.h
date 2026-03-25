#pragma once
#ifndef ENGINE_RHI_PIPELINE_H
#define ENGINE_RHI_PIPELINE_H

#include <vulkan/vulkan.h>
#include "Shader.h"
#include "RHITypes.h"
#include <vector>
#include <optional>

namespace engine::rhi {

// 顶点属性描述
struct VertexAttributeDesc {
    uint32_t location = 0;                          // Shader location
    uint32_t binding = 0;                           // Buffer binding
    VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;   // 数据格式
    uint32_t offset = 0;                            // 在顶点中的偏移
};

// 顶点绑定描述
struct VertexBindingDesc {
    uint32_t binding = 0;                           // Buffer binding
    uint32_t stride = 0;                            // 顶点大小
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
};

// 光栅化状态
struct RasterizerDesc {
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    float lineWidth = 1.0f;
    VkBool32 depthClampEnable = VK_FALSE;
    VkBool32 rasterizerDiscardEnable = VK_FALSE;
};

// 深度模板状态
struct DepthStencilDesc {
    VkBool32 depthTestEnable = VK_TRUE;
    VkBool32 depthWriteEnable = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    VkBool32 stencilTestEnable = VK_FALSE;
    VkStencilOpState front = {};
    VkStencilOpState back = {};
};

// 混合状态
struct BlendAttachmentDesc {
    VkBool32 blendEnable = VK_FALSE;
    VkBlendFactor srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
};

// 图形管线描述
struct GraphicsPipelineDesc {
    // Shader
    std::vector<ShaderHandle> shaders;              // Shader 句柄

    // Vertex Input
    std::vector<VertexBindingDesc> vertexBindings;
    std::vector<VertexAttributeDesc> vertexAttributes;

    // Input Assembly
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 primitiveRestartEnable = VK_FALSE;

    // Viewport & Scissor (动态状态)
    VkBool32 dynamicViewport = VK_TRUE;
    VkBool32 dynamicScissor = VK_TRUE;

    // Rasterizer
    RasterizerDesc rasterizer;

    // Multisample
    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth/Stencil
    DepthStencilDesc depthStencil;

    // Color Blend
    std::vector<BlendAttachmentDesc> colorBlendAttachments;
    float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Pipeline Layout
    PipelineLayoutHandle layout;

    // RenderPass
    VkRenderPass renderPass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    const char* name = nullptr;
};

// 计算管线描述
struct ComputePipelineDesc {
    ShaderHandle shader;                            // 计算 Shader
    PipelineLayoutHandle layout;                    // 管线布局
    const char* name = nullptr;
};

// 图形管线资源
struct GraphicsPipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;           // Vulkan Pipeline
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    [[nodiscard]] bool isValid() const { return pipeline != VK_NULL_HANDLE; }
};

// 计算管线资源
struct ComputePipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;           // Vulkan Pipeline
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    [[nodiscard]] bool isValid() const { return pipeline != VK_NULL_HANDLE; }
};

} // namespace engine::rhi

#endif // ENGINE_RHI_PIPELINE_H
