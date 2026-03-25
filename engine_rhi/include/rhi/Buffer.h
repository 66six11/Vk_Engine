#pragma once
#ifndef ENGINE_RHI_BUFFER_H
#define ENGINE_RHI_BUFFER_H

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <cstdint>

namespace engine::rhi {

// Buffer 描述信息
struct BufferDesc {
    VkDeviceSize size = 0;                          // 缓冲区大小
    VkBufferUsageFlags usage = 0;                   // 使用方式 (VERTEX, INDEX, UNIFORM, etc.)
    VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO; // 内存使用类型
    VkMemoryPropertyFlags requiredFlags = 0;        // 必需内存属性
    VkMemoryPropertyFlags preferredFlags = 0;       // 优先内存属性
    bool mapped = false;                            // 是否持久映射
    const char* name = nullptr;                     // 调试名称
};

// Buffer 资源结构
struct Buffer {
    VkBuffer buffer = VK_NULL_HANDLE;               // Vulkan Buffer 对象
    VmaAllocation allocation = VK_NULL_HANDLE;      // VMA 内存分配
    VmaAllocationInfo allocInfo = {};               // 分配信息
    VkDeviceSize size = 0;                          // 缓冲区大小
    VkBufferUsageFlags usage = 0;                   // 使用标志
    void* mappedData = nullptr;                     // CPU 映射指针（如启用持久映射）

    // 检查是否有效
    [[nodiscard]] bool isValid() const { return buffer != VK_NULL_HANDLE; }

    // 检查是否可映射到 CPU
    [[nodiscard]] bool isMappable() const {
        return allocInfo.pMappedData != nullptr || mappedData != nullptr;
    }

    // 获取 CPU 可写指针
    [[nodiscard]] void* getMappedPtr() const {
        return mappedData ? mappedData : allocInfo.pMappedData;
    }

    // 上传到 GPU（如果是 CPU 可访问内存）
    void uploadData(const void* data, VkDeviceSize offset, VkDeviceSize dataSize) const;

    // 从 GPU 读取数据
    void downloadData(void* dst, VkDeviceSize offset, VkDeviceSize dataSize) const;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_BUFFER_H
