#pragma once
#ifndef ENGINE_RHI_TEXTURE_MANAGER_H
#define ENGINE_RHI_TEXTURE_MANAGER_H

#include "../rhi/Texture.h"
#include "../core/ResourceManager.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace engine::rhi {

// 基础 Texture 管理器 - 只提供创建/销毁功能
// 复杂的上传和 Mipmap 生成由上层 RenderGraph 处理
class TextureManager : public ResourceManager<Texture, TextureHandle> {
public:
    TextureManager(VkDevice device, VmaAllocator allocator);
    ~TextureManager() override;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    // 创建 Texture（基础功能）
    [[nodiscard]] TextureHandle createTexture(const TextureDesc& desc);

    // 获取 Vulkan Image
    [[nodiscard]] VkImage getVkImage(TextureHandle handle) const {
        if (const Texture* tex = get(handle)) {
            return tex->image;
        }
        return VK_NULL_HANDLE;
    }

    // 获取 ImageView
    [[nodiscard]] VkImageView getVkImageView(TextureHandle handle) const {
        if (const Texture* tex = get(handle)) {
            return tex->view;
        }
        return VK_NULL_HANDLE;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_TEXTURE_MANAGER_H