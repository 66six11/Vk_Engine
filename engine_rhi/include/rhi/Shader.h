#pragma once
#ifndef ENGINE_RHI_SHADER_H
#define ENGINE_RHI_SHADER_H

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <string>

namespace engine::rhi {

// Shader 阶段
enum class ShaderStage : uint8_t {
    Vertex = 0,
    Fragment,
    Compute,
    Geometry,
    TessellationControl,
    TessellationEvaluation,
    Raygen,
    AnyHit,
    ClosestHit,
    Miss,
    Intersection,
    Callable,
    Task,
    Mesh,
    Count
};

// Shader 描述信息
struct ShaderDesc {
    ShaderStage stage = ShaderStage::Vertex;        // Shader 阶段
    const uint32_t* code = nullptr;                 // SPIR-V 代码指针
    size_t codeSize = 0;                            // 代码大小（字节）
    std::string entryPoint = "main";                // 入口函数名
    const char* name = nullptr;                     // 调试名称
};

// Shader 模块资源
struct Shader {
    VkShaderModule module = VK_NULL_HANDLE;         // Vulkan Shader 模块
    ShaderStage stage = ShaderStage::Vertex;        // 阶段
    std::string entryPoint = "main";                // 入口函数名

    // 检查是否有效
    [[nodiscard]] bool isValid() const { return module != VK_NULL_HANDLE; }

    // 获取 VkPipelineShaderStageCreateInfo
    [[nodiscard]] VkPipelineShaderStageCreateInfo getPipelineStageInfo() const;
};

// Shader 管线布局描述
struct PipelineLayoutDesc {
    std::vector<VkDescriptorSetLayout> setLayouts;  // 描述符集布局
    std::vector<VkPushConstantRange> pushConstants; // 推送常量范围
};

// Shader 管线布局资源
struct PipelineLayout {
    VkPipelineLayout layout = VK_NULL_HANDLE;       // Vulkan 管线布局

    [[nodiscard]] bool isValid() const { return layout != VK_NULL_HANDLE; }
};

} // namespace engine::rhi

#endif // ENGINE_RHI_SHADER_H
