//
// Created by C66 on 2026/3/31.
//
// CommandPool - Vulkan CommandPool 管理器
// 职责：分配/归还 CommandBuffer，支持批量重置
// 线程安全：支持多线程 Allocate/Free
//
#pragma once

#include "vulkan/Command/CommandBuffer.h"
#include <vulkan/vulkan.h>
#include <mutex>
#include <vector>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    // ==================== 配置 ====================

    struct CommandPoolConfig
    {
        uint32_t queueFamilyIndex = 0;
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        uint32_t initialBufferCount = 8;
    };

    // ==================== CommandPool ====================

    class CommandPool
    {
    public:
        CommandPool(DeviceManager& deviceManager, uint32_t queueFamilyIndex);
        ~CommandPool();

        // 禁止拷贝/移动
        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;
        CommandPool(CommandPool&&) = delete;
        CommandPool& operator=(CommandPool&&) = delete;

        // 初始化/销毁
        bool Initialize(const CommandPoolConfig& config);
        void Shutdown();

        // 分配 CommandBuffer（线程安全）
        CommandBuffer Allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // 归还 CommandBuffer（供 CommandBuffer 析构调用）
        void Free(VkCommandBuffer buffer, VkCommandBufferLevel level);

        // 重置整个池（调用者需确保无活跃 CommandBuffer）
        void Reset();

        // 访问器
        VkCommandPool GetHandle() const { return pool_; }
        bool IsInitialized() const { return pool_ != VK_NULL_HANDLE; }

    private:
        bool AllocateFromPool(uint32_t count, VkCommandBufferLevel level);

        DeviceManager& deviceManager_;
        uint32_t queueFamilyIndex_;

        VkCommandPool pool_ = VK_NULL_HANDLE;

        mutable std::mutex mutex_;
        std::vector<VkCommandBuffer> freePrimaryBuffers_;
        std::vector<VkCommandBuffer> freeSecondaryBuffers_;
    };

} // namespace engine::rhi::vulkan
