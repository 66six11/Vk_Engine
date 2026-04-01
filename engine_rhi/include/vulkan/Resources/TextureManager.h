#pragma once
#ifndef ENGINE_VULKAN_TEXTURE_MANAGER_H
#define ENGINE_VULKAN_TEXTURE_MANAGER_H

#include "Texture.h"
#include "VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <vector>

namespace engine::rhi::vulkan {

// TextureView条目（不暴露独立Handle，生命周期跟随Texture）
struct TextureViewEntry {
    VkImageView view = VK_NULL_HANDLE;
    TextureViewDesc desc;
    VulkanTextureHandle parentTexture;
    
    [[nodiscard]] bool isValid() const { return view != VK_NULL_HANDLE; }
};

// Texture管理器 - 合并TextureView管理
class TextureManager : public ResourceManager<Texture, VulkanTextureHandle> {
public:
    TextureManager(VkDevice device, VmaAllocator allocator);
    ~TextureManager() override;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    TextureManager(TextureManager&&) noexcept = default;
    TextureManager& operator=(TextureManager&&) noexcept = default;

    // Texture操作
    [[nodiscard]] VulkanTextureHandle createTexture(const TextureDesc& desc);
    [[nodiscard]] VkImage getVkImage(VulkanTextureHandle handle) const;
    [[nodiscard]] VkImageView getVkImageView(VulkanTextureHandle handle) const;
    [[nodiscard]] VkFormat getFormat(VulkanTextureHandle handle) const;

    // 更新lastKnownLayout（提示性）
    void updateLayout(VulkanTextureHandle handle, VkImageLayout layout);
    [[nodiscard]] VkImageLayout getLastKnownLayout(VulkanTextureHandle handle) const;

    // TextureView操作（合并，不暴露独立Handle）
    [[nodiscard]] VkImageView createTextureView(VulkanTextureHandle texture, const TextureViewDesc& desc);
    void destroyTextureView(VkImageView view);
    
    // 查询已有View（用于复用）
    [[nodiscard]] VkImageView findTextureView(VulkanTextureHandle texture, const TextureViewDesc& desc) const;

private:
    VkDevice m_device;
    VmaAllocator m_allocator;
    
    // 简单存储View，随Texture销毁自动清理
    std::vector<TextureViewEntry> m_textureViews;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_TEXTURE_MANAGER_H
