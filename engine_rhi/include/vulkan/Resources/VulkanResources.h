#pragma once
#ifndef ENGINE_VULKAN_RESOURCES_H
#define ENGINE_VULKAN_RESOURCES_H

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <cstdint>

namespace engine::rhi::vulkan {

// 统一资源描述符基类
struct VulkanResource {
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocInfo = {};

    [[nodiscard]] bool isAllocated() const { return allocation != VK_NULL_HANDLE; }
};

// 资源使用标志
enum class ResourceUsage : uint32_t {
    None = 0,

    // Buffer usage
    VertexBuffer = 1 << 0,
    IndexBuffer = 1 << 1,
    UniformBuffer = 1 << 2,
    StorageBuffer = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5,

    // Texture usage
    SampledTexture = 1 << 6,
    StorageTexture = 1 << 7,
    ColorAttachment = 1 << 8,
    DepthStencilAttachment = 1 << 9,
    InputAttachment = 1 << 10,

    // Memory location
    DeviceLocal = 1 << 16,  // GPU only
    HostVisible = 1 << 17,  // CPU accessible
    HostCached = 1 << 18,   // CPU cached
};

constexpr ResourceUsage operator|(ResourceUsage a, ResourceUsage b) {
    return static_cast<ResourceUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr bool hasUsage(ResourceUsage flags, ResourceUsage flag) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

// ResourceUsage 转换为 VkBufferUsageFlags
inline VkBufferUsageFlags convertBufferUsage(ResourceUsage usage) {
    VkBufferUsageFlags result = 0;
    if (hasUsage(usage, ResourceUsage::VertexBuffer))   result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (hasUsage(usage, ResourceUsage::IndexBuffer))    result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (hasUsage(usage, ResourceUsage::UniformBuffer))  result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (hasUsage(usage, ResourceUsage::StorageBuffer))  result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (hasUsage(usage, ResourceUsage::TransferSrc))    result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (hasUsage(usage, ResourceUsage::TransferDst))    result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    return result;
}

// ResourceUsage 转换为 VkImageUsageFlags
inline VkImageUsageFlags convertImageUsage(ResourceUsage usage) {
    VkImageUsageFlags result = 0;
    if (hasUsage(usage, ResourceUsage::SampledTexture))       result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (hasUsage(usage, ResourceUsage::StorageTexture))       result |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (hasUsage(usage, ResourceUsage::ColorAttachment))      result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (hasUsage(usage, ResourceUsage::DepthStencilAttachment)) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (hasUsage(usage, ResourceUsage::InputAttachment))      result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if (hasUsage(usage, ResourceUsage::TransferSrc))          result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (hasUsage(usage, ResourceUsage::TransferDst))          result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    return result;
}

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_RESOURCES_H
