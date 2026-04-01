#pragma once
#ifndef ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H
#define ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H

#include "../Resources/VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace engine::rhi::vulkan {

// PipelineLayout描述（简化，Bindless架构通常只有一个全局Set）
struct PipelineLayoutDesc {
    VkDescriptorSetLayout globalSetLayout = VK_NULL_HANDLE;  // Bindless全局Set
    std::vector<VkPushConstantRange> pushConstants;
    
    // 用于相等比较
    [[nodiscard]] bool operator==(const PipelineLayoutDesc& other) const {
        if (globalSetLayout != other.globalSetLayout) return false;
        if (pushConstants.size() != other.pushConstants.size()) return false;
        
        for (size_t i = 0; i < pushConstants.size(); ++i) {
            const auto& a = pushConstants[i];
            const auto& b = other.pushConstants[i];
            if (a.stageFlags != b.stageFlags ||
                a.offset != b.offset ||
                a.size != b.size) {
                return false;
            }
        }
        return true;
    }
    
    [[nodiscard]] uint64_t computeHash() const;
};

// PipelineLayout资源
struct PipelineLayout {
    VkPipelineLayout layout = VK_NULL_HANDLE;
    PipelineLayoutDesc desc;
    
    [[nodiscard]] bool isValid() const { return layout != VK_NULL_HANDLE; }
};

// PipelineLayout管理器 - 带缓存，适配Bindless架构
class PipelineLayoutManager : public ResourceManager<PipelineLayout, VulkanPipelineLayoutHandle> {
public:
    explicit PipelineLayoutManager(VkDevice device);
    ~PipelineLayoutManager() override;

    PipelineLayoutManager(const PipelineLayoutManager&) = delete;
    PipelineLayoutManager& operator=(const PipelineLayoutManager&) = delete;

    PipelineLayoutManager(PipelineLayoutManager&&) noexcept = default;
    PipelineLayoutManager& operator=(PipelineLayoutManager&&) noexcept = default;

    // 获取或创建（自动缓存去重）
    [[nodiscard]] VulkanPipelineLayoutHandle getOrCreateLayout(const PipelineLayoutDesc& desc);
    
    // 便捷方法：Bindless架构专用（单Set）
    [[nodiscard]] VulkanPipelineLayoutHandle getOrCreateBindlessLayout(
        VkDescriptorSetLayout globalSetLayout,
        const std::vector<VkPushConstantRange>& pushConstants = {});

    // 获取Vulkan原始句柄
    [[nodiscard]] VkPipelineLayout getVkPipelineLayout(VulkanPipelineLayoutHandle handle) const;

    // 缓存统计
    [[nodiscard]] size_t getCacheSize() const { return m_layoutCache.size(); }
    void clearCache();

private:
    // 创建Layout（私有辅助方法）
    [[nodiscard]] VulkanPipelineLayoutHandle createLayout(const PipelineLayoutDesc& desc);

    VkDevice m_device;
    std::unordered_map<uint64_t, VulkanPipelineLayoutHandle> m_layoutCache;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_PIPELINE_LAYOUT_MANAGER_H