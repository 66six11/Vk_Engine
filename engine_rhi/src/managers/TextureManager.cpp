#include "../../include/managers/TextureManager.h"
#include <cassert>

namespace engine::rhi {

TextureManager::TextureManager(VkDevice device, VmaAllocator allocator)
    : m_device(device), m_allocator(allocator) {
    assert(device != VK_NULL_HANDLE);
    assert(allocator != VK_NULL_HANDLE);
}

TextureManager::~TextureManager() {
    for (auto it = begin(); it != end(); ++it) {
        Texture& texture = *it;
        if (texture.view != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, texture.view, nullptr);
        }
        if (texture.image != VK_NULL_HANDLE) {
            vmaDestroyImage(m_allocator, texture.image, texture.allocation);
        }
    }
    clear();
}

TextureHandle TextureManager::createTexture(const TextureDesc& desc) {
    // 创建 Image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = desc.format.width;
    imageInfo.extent.height = desc.format.height;
    imageInfo.extent.depth = desc.format.depth;
    imageInfo.mipLevels = desc.format.mipLevels;
    imageInfo.arrayLayers = desc.format.arrayLayers;
    imageInfo.format = desc.format.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = desc.usage;
    imageInfo.samples = desc.format.samples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    Texture texture = {};
    texture.format = desc.format;
    texture.type = desc.type;

    VkResult result = vmaCreateImage(m_allocator, &imageInfo, &allocInfo,
                                     &texture.image, &texture.allocation, nullptr);
    if (result != VK_SUCCESS) {
        return TextureHandle();
    }

    // 创建 ImageView
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.image;
    viewInfo.viewType = (desc.format.arrayLayers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = desc.format.format;
    viewInfo.subresourceRange.aspectMask = desc.viewAspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = desc.format.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = desc.format.arrayLayers;

    result = vkCreateImageView(m_device, &viewInfo, nullptr, &texture.view);
    if (result != VK_SUCCESS) {
        vmaDestroyImage(m_allocator, texture.image, texture.allocation);
        return TextureHandle();
    }

    return create(std::move(texture));
}

} // namespace engine::rhi
