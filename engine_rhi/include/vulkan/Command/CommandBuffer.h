//
// Created by C66 on 2026/3/31.
//
// CommandBuffer - Vulkan CommandBuffer 轻量级 RAII 包装
// 职责：生命周期管理（自动归还到 Pool），Begin/End 录制
// 注意：不封装具体 vkCmd* 命令，保持 Thin Wrapper
//
//
// Created by C66 on 2026/3/31.
//
// CommandBuffer - Vulkan CommandBuffer 轻量级 RAII 包装
// 职责：生命周期管理（自动归还到 Pool），Begin/End 录制
// 注意：不封装具体 vkCmd* 命令，保持 Thin Wrapper
//
#pragma once

#include <vulkan/vulkan.h>

namespace engine::rhi::vulkan
{
    // 前置声明
    class CommandPool;

    // ==================== CommandBuffer ====================

    class CommandBuffer
    {
    public:
        CommandBuffer();
        // 构造函数：Pool 内部调用，不直接构造
        CommandBuffer(VkCommandBuffer handle, CommandPool* pool);
        ~CommandBuffer();

        // 禁止拷贝
        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        // 允许移动
        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        // 录制控制
        bool Begin(VkCommandBufferUsageFlags usage = 0);
        bool End();

        // 查询状态
        bool IsRecording() const { return isRecording_; }
        bool IsValid() const { return handle_ != VK_NULL_HANDLE; }

        // 访问底层句柄（用于直接调用 vkCmd*）
        VkCommandBuffer GetHandle() const { return handle_; }
        operator VkCommandBuffer() const { return handle_; }

    private:
        void Reset();

        VkCommandBuffer handle_ = VK_NULL_HANDLE;
        CommandPool*    pool_   = nullptr;
        bool            isRecording_ = false;
    };

} // namespace engine::rhi::vulkan