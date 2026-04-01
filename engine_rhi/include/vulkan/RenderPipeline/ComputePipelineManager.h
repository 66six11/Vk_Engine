#pragma once
#ifndef ENGINE_VULKAN_COMPUTE_PIPELINE_MANAGER_H
#define ENGINE_VULKAN_COMPUTE_PIPELINE_MANAGER_H

#include "ComputePipeline.h"
#include "../Resources/VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"

namespace engine::rhi::vulkan
{
    // 计算管线管理器 - 只提供基础创建/销毁
    class ComputePipelineManager : public ResourceManager<ComputePipeline, VulkanComputePipelineHandle>
    {
        public:
            explicit ComputePipelineManager(VkDevice device);
            ~ComputePipelineManager() override;

            ComputePipelineManager(const ComputePipelineManager&)            = delete;
            ComputePipelineManager& operator=(const ComputePipelineManager&) = delete;

            ComputePipelineManager(ComputePipelineManager&&) noexcept            = default;
            ComputePipelineManager& operator=(ComputePipelineManager&&) noexcept = default;

            // 基础创建/销毁
            [[nodiscard]] VulkanComputePipelineHandle createPipeline(const ComputePipelineDesc& desc);

            // 获取Vulkan原始句柄
            [[nodiscard]] VkPipeline       getVkPipeline(VulkanComputePipelineHandle handle) const;
            [[nodiscard]] VkPipelineLayout getVkPipelineLayout(VulkanComputePipelineHandle handle) const;

            // 获取描述（上层可用于计算缓存key）
            [[nodiscard]] const ComputePipelineDesc* getDesc(VulkanComputePipelineHandle handle) const;

        private:
            VkDevice m_device;
    };
} // namespace engine::rhi::vulkan


#endif // ENGINE_VULKAN_COMPUTE_PIPELINE_MANAGER_H
