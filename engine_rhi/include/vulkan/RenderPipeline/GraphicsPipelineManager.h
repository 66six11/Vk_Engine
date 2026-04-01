#pragma once
#ifndef ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H
#define ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H

#include "GraphicsPipeline.h"
#include "../Resources/VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"

namespace engine::rhi::vulkan {

// 图形管线管理器 - 强制使用 Dynamic Rendering (Vulkan 1.3+)
// 注意：shaders字段必须包含有效的VkShaderModule（由上层提供）
class GraphicsPipelineManager : public ResourceManager<GraphicsPipeline, VulkanGraphicsPipelineHandle> {
public:
    GraphicsPipelineManager(VkDevice device, VkPipelineCache pipelineCache);
    ~GraphicsPipelineManager() override;

    GraphicsPipelineManager(const GraphicsPipelineManager&) = delete;
    GraphicsPipelineManager& operator=(const GraphicsPipelineManager&) = delete;

    GraphicsPipelineManager(GraphicsPipelineManager&&) noexcept = default;
    GraphicsPipelineManager& operator=(GraphicsPipelineManager&&) noexcept = default;

    // 基础创建/销毁（上层决定缓存策略）
    // 注意：desc.shaders中的module必须是有效的VkShaderModule
    [[nodiscard]] VulkanGraphicsPipelineHandle createPipeline(const GraphicsPipelineDesc& desc);

    // 获取Vulkan原始句柄
    [[nodiscard]] VkPipeline getVkPipeline(VulkanGraphicsPipelineHandle handle) const;
    [[nodiscard]] VkPipelineLayout getVkPipelineLayout(VulkanGraphicsPipelineHandle handle) const;

    // 获取描述（上层可用于计算缓存key）
    [[nodiscard]] const GraphicsPipelineDesc* getDesc(VulkanGraphicsPipelineHandle handle) const;

    // 获取Pipeline Cache（用于序列化到磁盘）
    [[nodiscard]] VkPipelineCache getPipelineCache() const { return m_pipelineCache; }

private:
    VkDevice m_device;
    VkPipelineCache m_pipelineCache;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_GRAPHICS_PIPELINE_MANAGER_H
