#pragma once
#ifndef ENGINE_VULKAN_SHADER_H
#define ENGINE_VULKAN_SHADER_H

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>

namespace engine::rhi::vulkan {

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

struct ShaderDesc {
    ShaderStage stage = ShaderStage::Vertex;
    const uint32_t* code = nullptr;  // SPIR-V字节码
    size_t codeSize = 0;             // 字节数（不是uint32_t个数）
    std::string entryPoint = "main";
    const char* debugName = nullptr;
};

struct Shader {
    VkShaderModule module = VK_NULL_HANDLE;
    ShaderStage stage = ShaderStage::Vertex;
    std::string entryPoint = "main";

    [[nodiscard]] bool isValid() const { return module != VK_NULL_HANDLE; }

    // 获取PipelineShaderStageCreateInfo（实现在cpp中）
    [[nodiscard]] VkPipelineShaderStageCreateInfo getPipelineStageInfo() const;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_SHADER_H
