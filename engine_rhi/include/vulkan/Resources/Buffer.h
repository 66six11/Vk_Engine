#pragma once
#ifndef ENGINE_VULKAN_BUFFER_H
#define ENGINE_VULKAN_BUFFER_H

#include "VulkanResources.h"

namespace engine::rhi::vulkan
{
    struct BufferDesc
    {
        uint64_t      size          = 0;
        uint32_t      stride        = 0; // 结构化buffer用，0表示非结构化
        ResourceUsage usage         = ResourceUsage::None;
        bool          persistentMap = false; // 是否保持CPU映射
        const char*   debugName     = nullptr;
    };

    struct Buffer : public VulkanResource
    {
        VkBuffer      buffer    = VK_NULL_HANDLE;
        uint64_t      size      = 0;
        uint32_t      stride    = 0;
        ResourceUsage usage     = ResourceUsage::None;
        void*         mappedPtr = nullptr; // 仅当persistentMap=true时有效

        [[nodiscard]] bool  isValid() const { return buffer != VK_NULL_HANDLE; }
        [[nodiscard]] bool  isMappable() const { return mappedPtr != nullptr || allocInfo.pMappedData != nullptr; }
        [[nodiscard]] void* getMappedPtr() const { return mappedPtr ? mappedPtr : allocInfo.pMappedData; }

        // 获取设备地址（用于RT或bindless）
        [[nodiscard]] VkDeviceAddress getDeviceAddress(VkDevice device) const;
    };

    // BufferView - 用于格式化buffer视图（如 UAV）
    struct BufferViewDesc
    {
        VkFormat format = VK_FORMAT_UNDEFINED;
        uint64_t offset = 0;
        uint64_t range  = 0;
    };

    struct BufferView
    {
        VkBufferView   view = VK_NULL_HANDLE;
        BufferViewDesc desc;

        [[nodiscard]] bool isValid() const { return view != VK_NULL_HANDLE; }
    };
} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_BUFFER_H
