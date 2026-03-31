//
// Created by C66 on 2026/3/29.
//
// Semaphore 对象池 - 仅用于二进制信号量
//
#pragma once

#include "vulkan/Synchronization/Semaphore.h"
#include "vulkan/Synchronization/SyncConfig.h"
#include <mutex>
#include <vector>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    // 二进制 Semaphore 对象池
    class SemaphorePool
    {
        public:
            explicit SemaphorePool(DeviceManager& deviceManager);
            ~SemaphorePool();

            // 禁止拷贝/移动
            SemaphorePool(const SemaphorePool&)            = delete;
            SemaphorePool& operator=(const SemaphorePool&) = delete;
            SemaphorePool(SemaphorePool&&)                 = delete;
            SemaphorePool& operator=(SemaphorePool&&)      = delete;

            // 初始化/销毁
            bool Initialize(const SemaphorePoolConfig& config);
            void Shutdown();

            // 获取/归还 Semaphore
            Semaphore Acquire();
            void      Release(VkSemaphore semaphore);

            // 统计
            uint32_t GetAvailableCount() const { return static_cast<uint32_t>(availableSemaphores_.size()); }
            uint32_t GetActiveCount() const { return activeCount_; }
            uint32_t GetTotalCount() const { return totalCount_; }

        private:
            bool CreateSemaphore(VkSemaphore& outSemaphore);

            DeviceManager&      deviceManager_;
            SemaphorePoolConfig config_;

            mutable std::mutex       mutex_;
            std::vector<VkSemaphore> availableSemaphores_;
            uint32_t                 activeCount_ = 0;
            uint32_t                 totalCount_  = 0;
    };
} // namespace engine::rhi::vulkan
