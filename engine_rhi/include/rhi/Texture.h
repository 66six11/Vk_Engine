#pragma once
#ifndef ENGINE_RHI_TEXTURE_H
#define ENGINE_RHI_TEXTURE_H

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <cstdint>

namespace engine::rhi {

// 纹理类型
enum class TextureType : uint8_t {
    Texture2D,
    Texture3D,
    TextureCube,
    Texture2DArray,
    TextureCubeArray,
};

// 纹理格式信息
struct TextureFormat {
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;             // 3D 纹理深度，数组纹理层数
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
};

// 纹理描述信息
struct TextureDesc {
    TextureFormat format;                           // 格式信息
    TextureType type = TextureType::Texture2D;      // 纹理类型
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // 使用方式
    VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO;   // 内存使用类型
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool createView = true;                         // 是否自动创建 ImageView
    VkImageAspectFlags viewAspect = VK_IMAGE_ASPECT_COLOR_BIT;
    const char* name = nullptr;                     // 调试名称
};

// 纹理资源结构
struct Texture {
    VkImage image = VK_NULL_HANDLE;                 // Vulkan Image 对象
    VkImageView view = VK_NULL_HANDLE;              // ImageView
    VmaAllocation allocation = VK_NULL_HANDLE;      // VMA 内存分配
    VmaAllocationInfo allocInfo = {};               // 分配信息
    TextureFormat format;                           // 格式信息
    TextureType type = TextureType::Texture2D;      // 纹理类型
    VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 当前布局

    // 检查是否有效
    [[nodiscard]] bool isValid() const { return image != VK_NULL_HANDLE; }

    // 获取完整图像范围
    [[nodiscard]] VkImageSubresourceRange getFullRange() const;

    // 转换布局
    void transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);
};

// 获取 TextureType 对应的 VkImageViewType
[[nodiscard]] inline VkImageViewType getViewType(TextureType type) {
    switch (type) {
        case TextureType::Texture2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case TextureType::Texture3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        case TextureType::TextureCube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case TextureType::Texture2DArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TextureType::TextureCubeArray:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    }
    return VK_IMAGE_VIEW_TYPE_2D;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_TEXTURE_H
