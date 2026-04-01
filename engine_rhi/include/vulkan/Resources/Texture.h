#pragma once
#ifndef ENGINE_VULKAN_TEXTURE_H
#define ENGINE_VULKAN_TEXTURE_H

#include "VulkanResources.h"

namespace engine::rhi::vulkan
{
    // 纹理类型由VkImageViewType决定，不单独分类
    struct TextureDesc
    {
        VkExtent3D            extent      = {}; // width/height/depth，2D时depth=1
        VkFormat              format      = VK_FORMAT_UNDEFINED;
        uint32_t              mipLevels   = 1;
        uint32_t              arrayLayers = 1;
        VkSampleCountFlagBits samples     = VK_SAMPLE_COUNT_1_BIT;
        VkImageViewType       viewType    = VK_IMAGE_VIEW_TYPE_2D; // 2D/3D/Cube/Array
        ResourceUsage         usage       = ResourceUsage::SampledTexture;
        const char*           debugName   = nullptr;

        // 便捷方法
        [[nodiscard]] bool is2D() const { return viewType == VK_IMAGE_VIEW_TYPE_2D; }
        [[nodiscard]] bool is3D() const { return viewType == VK_IMAGE_VIEW_TYPE_3D; }
        [[nodiscard]] bool isCube() const { return viewType == VK_IMAGE_VIEW_TYPE_CUBE || viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; }

        [[nodiscard]] bool isArray() const
        {
            return viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY ||
                   viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ||
                   viewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
    };

    struct Texture : public VulkanResource
    {
        VkImage     image = VK_NULL_HANDLE;
        VkImageView view  = VK_NULL_HANDLE; // 默认全范围view

        TextureDesc desc;

        // 注意：这是"上次已知布局"，非权威状态
        // 权威状态由上层（RenderGraph）管理
        VkImageLayout lastKnownLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        [[nodiscard]] bool isValid() const { return image != VK_NULL_HANDLE; }

        // 获取完整subresource range
        [[nodiscard]] VkImageSubresourceRange getFullRange(VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) const
        {
            VkImageSubresourceRange range = {};
            range.aspectMask              = aspect;
            range.baseMipLevel            = 0;
            range.levelCount              = desc.mipLevels;
            range.baseArrayLayer          = 0;
            range.layerCount              = desc.arrayLayers;
            return range;
        }
    };

    // 额外的TextureView - 当需要非默认view时使用（如指定mipmap层级）
    struct TextureViewDesc
    {
        VkFormat           format          = VK_FORMAT_UNDEFINED; // 可重解释格式
        VkImageViewType    viewType        = VK_IMAGE_VIEW_TYPE_2D;
        uint32_t           baseMipLevel    = 0;
        uint32_t           mipLevelCount   = 1;
        uint32_t           baseArrayLayer  = 0;
        uint32_t           arrayLayerCount = 1;
        VkImageAspectFlags aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    };

    struct TextureView
    {
        VkImageView     view = VK_NULL_HANDLE;
        TextureViewDesc desc;

        [[nodiscard]] bool isValid() const { return view != VK_NULL_HANDLE; }
    };
} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_TEXTURE_H
