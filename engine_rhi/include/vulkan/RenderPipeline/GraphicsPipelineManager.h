#pragma once
#ifndef ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H
#define ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H

#include "GraphicsPipeline.h"
#include "../Resources/VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"

namespace engine::rhi::vulkan {

// 图形管线管理器 - 强制使用 Dynamic Rendering (Vulkan 1.3+)
// 缓存策略交给上层RenderGraph决定
class GraphicsPipelineManager : public ResourceManager<GraphicsPipeline, VulkanGraphicsPipelineHandle> {
public:
    explicit GraphicsPipelineManager(VkDevice device);
    ~GraphicsPipelineManager() override;

    GraphicsPipelineManager(const GraphicsPipelineManager&) = delete;
    GraphicsPipelineManager& operator=(const GraphicsPipelineManager&) = delete;

    GraphicsPipelineManager(GraphicsPipelineManager&&) noexcept = default;
    GraphicsPipelineManager& operator=(GraphicsPipelineManager&&) noexcept = default;

    // 基础创建/销毁（上层决定缓存策略）
    [[nodiscard]] VulkanGraphicsPipelineHandle createPipeline(const GraphicsPipelineDesc& desc);

    // 获取Vulkan原始句柄
    [[nodiscard]] VkPipeline getVkPipeline(VulkanGraphicsPipelineHandle handle) const;
    [[nodiscard]] VkPipelineLayout getVkPipelineLayout(VulkanGraphicsPipelineHandle handle) const;

    // 获取描述（上层可用于计算缓存key）
    [[nodiscard]] const GraphicsPipelineDesc* getDesc(VulkanGraphicsPipelineHandle handle) const;

private:
    VkDevice m_device;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H
