//
// Created by C66 on 2026/3/28.
//
#pragma once

#include <functional>
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace engine::rhi::vulkan
{
    class DeviceManager;
    class SynchronizationManager;
    
    // 命令池配置
    struct CommandPoolConfig
    {
        uint32_t                 queueFamilyIndex   = 0;                                               // 队列族索引
        VkCommandPoolCreateFlags flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // 池创建标志
        uint32_t                 initialBufferCount = 8;                                               // 初始缓冲数量
    };

    // 命令缓冲包装
    struct CommandBuffer
    {
        VkCommandBuffer      handle      = VK_NULL_HANDLE;                  // Vulkan 句柄
        VkCommandPool        pool        = VK_NULL_HANDLE;                  // 所属池
        VkCommandBufferLevel level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // 命令缓冲级别
        bool                 isRecording = false;                           // 是否正在录制

        // 便捷操作
        void Begin(VkCommandBufferUsageFlags usage = 0);
        void End();
        void Reset();

        explicit operator bool() const { return handle != VK_NULL_HANDLE; }
    };

    // 命令缓冲管理器
    class CommandBufferManager
    {
        public:
            explicit CommandBufferManager(DeviceManager& deviceManager);
            ~CommandBufferManager();

            // 禁止拷贝
            CommandBufferManager(const CommandBufferManager&)            = delete;
            CommandBufferManager& operator=(const CommandBufferManager&) = delete;

            // 移动语义
            CommandBufferManager(CommandBufferManager&& other) noexcept;
            CommandBufferManager& operator=(CommandBufferManager&& other) noexcept;

            // 初始化/销毁
            bool Initialize(const CommandPoolConfig& config);
            void Shutdown();

            // 分配命令缓冲
            CommandBuffer AllocatePrimary();
            CommandBuffer AllocateSecondary();

            // 批量分配
            std::vector<CommandBuffer> AllocatePrimary(uint32_t count);
            std::vector<CommandBuffer> AllocateSecondary(uint32_t count);

            // 释放命令缓冲（归还池中）
            void Free(const CommandBuffer& cmdBuffer);
            void Free(const std::vector<CommandBuffer>& cmdBuffers);

            // 重置整个池（所有命令缓冲失效）
            void ResetPool();

            // 提交命令缓冲（便捷接口）
            void Submit(
                VkQueue              queue,
                const CommandBuffer& cmdBuffer,
                VkSemaphore          waitSemaphore   = VK_NULL_HANDLE,
                VkSemaphore          signalSemaphore = VK_NULL_HANDLE,
                VkFence              fence           = VK_NULL_HANDLE
            );

            // 批量提交
            void Submit(
                VkQueue                           queue,
                const std::vector<CommandBuffer>& cmdBuffers,
                VkSemaphore                       waitSemaphore   = VK_NULL_HANDLE,
                VkSemaphore                       signalSemaphore = VK_NULL_HANDLE,
                VkFence                           fence           = VK_NULL_HANDLE
            );

            // One-shot 执行（分配-录制-提交-释放）
            void ExecuteOneShot(
                VkQueue                                  queue,
                std::function<void(VkCommandBuffer cmd)> recordFunc
            );

            // Getter
            VkCommandPool GetPool() const { return pool_; }
            uint32_t      GetAllocatedCount() const { return allocatedCount_; }
            uint32_t      GetActiveCount() const { return activeCount_; }

        private:
            DeviceManager& deviceManager_;

            VkCommandPool     pool_ = VK_NULL_HANDLE;
            CommandPoolConfig config_;

            // 统计
            uint32_t allocatedCount_ = 0;
            uint32_t activeCount_    = 0;

            // 空闲列表（简单实现）
            std::vector<VkCommandBuffer> freePrimaryBuffers_;
            std::vector<VkCommandBuffer> freeSecondaryBuffers_;

            bool AllocateFromPool(uint32_t count, VkCommandBufferLevel level, std::vector<VkCommandBuffer>& outBuffers);
    };
} // namespace engine::rhi::vulkan
