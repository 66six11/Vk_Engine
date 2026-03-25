#pragma once
#ifndef ENGINE_RHI_BUFFER_MANAGER_H
#define ENGINE_RHI_BUFFER_MANAGER_H

#include "../rhi/Buffer.h"
#include "../core/ResourceManager.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace engine::rhi {

// 基础 Buffer 管理器 - 只提供创建/销毁功能
// 复杂的上传逻辑由上层 RenderGraph 或 RenderSystem 处理
class BufferManager : public ResourceManager<Buffer, BufferHandle> {
public:
    BufferManager(VkDevice device, VmaAllocator allocator);
    ~BufferManager() override;

    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;

    // 创建 Buffer（基础功能）
    [[nodiscard]] BufferHandle createBuffer(const BufferDesc& desc);

    // 获取 Vulkan Buffer
    [[nodiscard]] VkBuffer getVkBuffer(BufferHandle handle) const {
        if (const Buffer* buffer = get(handle)) {
            return buffer->buffer;
        }
        return VK_NULL_HANDLE;
    }

    // 获取 Buffer 地址（用于 RT）
    [[nodiscard]] VkDeviceAddress getBufferAddress(BufferHandle handle) const;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_BUFFER_MANAGER_H