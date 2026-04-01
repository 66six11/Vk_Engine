#pragma once
#ifndef ENGINE_VULKAN_BUFFER_MANAGER_H
#define ENGINE_VULKAN_BUFFER_MANAGER_H

#include "Buffer.h"
#include "VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <vector>

namespace engine::rhi::vulkan {

// BufferView条目（不暴露独立Handle，生命周期跟随Buffer）
struct BufferViewEntry {
    VkBufferView view = VK_NULL_HANDLE;
    BufferViewDesc desc;
    VulkanBufferHandle parentBuffer;
    
    [[nodiscard]] bool isValid() const { return view != VK_NULL_HANDLE; }
};

// Buffer管理器 - 合并BufferView管理
class BufferManager : public ResourceManager<Buffer, VulkanBufferHandle> {
public:
    BufferManager(VkDevice device, VmaAllocator allocator);
    ~BufferManager() override;

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    BufferManager(BufferManager&&) noexcept = default;
    BufferManager& operator=(BufferManager&&) noexcept = default;

    // Buffer操作
    [[nodiscard]] VulkanBufferHandle createBuffer(const BufferDesc& desc);
    [[nodiscard]] VkBuffer getVkBuffer(VulkanBufferHandle handle) const;
    [[nodiscard]] VkDeviceAddress getDeviceAddress(VulkanBufferHandle handle) const;
    [[nodiscard]] void* getMappedPtr(VulkanBufferHandle handle) const;
    [[nodiscard]] bool isMappable(VulkanBufferHandle handle) const;

    // BufferView操作（合并，不暴露独立Handle）
    [[nodiscard]] VkBufferView createBufferView(VulkanBufferHandle buffer, const BufferViewDesc& desc);
    void destroyBufferView(VkBufferView view);
    
    // 查询已有View（用于复用）
    [[nodiscard]] VkBufferView findBufferView(VulkanBufferHandle buffer, const BufferViewDesc& desc) const;

private:
    VkDevice m_device;
    VmaAllocator m_allocator;
    
    // 简单存储View，随Buffer销毁自动清理
    std::vector<BufferViewEntry> m_bufferViews;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_BUFFER_MANAGER_H
