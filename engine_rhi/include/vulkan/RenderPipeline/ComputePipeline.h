#pragma once
#ifndef ENGINE_VULKAN_COMPUTE_PIPELINE_H
#define ENGINE_VULKAN_COMPUTE_PIPELINE_H

#include <vulkan/vulkan.h>
#include "../Resources/VulkanResourceHandles.h"

namespace engine::rhi::vulkan {

// 计算管线描述 - 比图形管线简单很多
struct ComputePipelineDesc {
    VulkanShaderHandle computeShader;           // 计算shader
    VulkanPipelineLayoutHandle layout;          // pipeline layout
    const char* debugName = nullptr;
    
    // 计算管线通常不需要复杂的状态配置
    // 所有执行配置在dispatch时指定
};

// 计算管线资源
struct ComputePipeline {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    ComputePipelineDesc desc;
    
    [[nodiscard]] bool isValid() const { return pipeline != VK_NULL_HANDLE; }
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_COMPUTE_PIPELINE_H
