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

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_RESOURCES_H
