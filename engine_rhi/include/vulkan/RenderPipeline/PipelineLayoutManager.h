#pragma once
#ifndef ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H
#define ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H

#include "../Resources/VulkanResourceHandles.h"
#include "../Resources/Shader.h"
#include "../../core/ResourceManager.h"
#include <vector>

namespace engine::rhi::vulkan {

// PipelineLayout描述
struct PipelineLayoutDesc {
    std::vector<VkDescriptorSetLayout> setLayouts;
    std::vector<VkPushConstantRange> pushConstants;
};

// PipelineLayout资源
struct PipelineLayout {
    VkPipelineLayout layout = VK_NULL_HANDLE;

    [[nodiscard]] bool isValid() const { return layout != VK_NULL_HANDLE; }
};

// PipelineLayout管理器
class PipelineLayoutManager : public ResourceManager<PipelineLayout, VulkanPipelineLayoutHandle> {
public:
    explicit PipelineLayoutManager(VkDevice device);
    ~PipelineLayoutManager() override;

    PipelineLayoutManager(const PipelineLayoutManager&) = delete;
    PipelineLayoutManager& operator=(const PipelineLayoutManager&) = delete;

    PipelineLayoutManager(PipelineLayoutManager&&) noexcept = default;
    PipelineLayoutManager& operator=(PipelineLayoutManager&&) noexcept = default;

    // 创建PipelineLayout
    [[nodiscard]] VulkanPipelineLayoutHandle createPipelineLayout(const PipelineLayoutDesc& desc);

    // 便捷方法：从单个DescriptorSetLayout创建
    [[nodiscard]] VulkanPipelineLayoutHandle createPipelineLayout(
        VkDescriptorSetLayout setLayout,
        const std::vector<VkPushConstantRange>& pushConstants = {});

    // 获取Vulkan原始句柄
    [[nodiscard]] VkPipelineLayout getVkPipelineLayout(VulkanPipelineLayoutHandle handle) const;

private:
    VkDevice m_device;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H
