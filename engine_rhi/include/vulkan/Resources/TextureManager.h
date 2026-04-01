#pragma once
#ifndef ENGINE_VULKAN_TEXTURE_MANAGER_H
#define ENGINE_VULKAN_TEXTURE_MANAGER_H

#include "Texture.h"
#include "VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <vector>
#include <unordered_map>

namespace engine::rhi::vulkan {

// 前向声明
class BindlessDescriptorManager;

// TextureView条目（不暴露独立Handle，生命周期跟随Texture）
struct TextureViewEntry {
    VkImageView view = VK_NULL_HANDLE;
    TextureViewDesc desc;
    VulkanTextureHandle parentTexture;
    
    [[nodiscard]] bool isValid() const { return view != VK_NULL_HANDLE; }
};

// Texture管理器 - 支持Bindless架构
class TextureManager : public ResourceManager<Texture, VulkanTextureHandle> {
public:
    TextureManager(VkDevice device, VmaAllocator allocator);
    ~TextureManager() override;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager(TextureManager&&) noexcept = default;
    TextureManager& operator=(TextureManager&&) noexcept = default;

    // 设置BindlessManager（启用Bindless时必须设置）
    void setBindlessManager(BindlessDescriptorManager* bindless);

    // Texture操作
    [[nodiscard]] VulkanTextureHandle createTexture(const TextureDesc& desc);
    [[nodiscard]] VulkanTextureHandle createTexture(const TextureDesc& desc, VkSampler sampler);
    [[nodiscard]] VkImage getVkImage(VulkanTextureHandle handle) const;
    [[nodiscard]] VkImageView getVkImageView(VulkanTextureHandle handle) const;
    [[nodiscard]] VkFormat getFormat(VulkanTextureHandle handle) const;

    // Bindless支持
    [[nodiscard]] uint32_t getBindlessIndex(VulkanTextureHandle handle) const;
    [[nodiscard]] bool isBindlessEnabled() const { return m_bindless != nullptr; }

    // 更新lastKnownLayout（提示性）
    void updateLayout(VulkanTextureHandle handle, VkImageLayout layout);
    [[nodiscard]] VkImageLayout getLastKnownLayout(VulkanTextureHandle handle) const;

    // TextureView操作
    [[nodiscard]] VkImageView createTextureView(VulkanTextureHandle texture, const TextureViewDesc& desc);
    void destroyTextureView(VkImageView view);
    [[nodiscard]] VkImageView findTextureView(VulkanTextureHandle texture, const TextureViewDesc& desc) const;

private:
    VkDevice m_device;
    VmaAllocator m_allocator;
    BindlessDescriptorManager* m_bindless = nullptr;
    
    // Texture到Bindless索引的映射
    std::unordered_map<uint32_t, uint32_t> m_textureToBindlessIndex;
    
    // 简单存储View
    std::vector<TextureViewEntry> m_textureViews;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_TEXTURE_MANAGER_H
