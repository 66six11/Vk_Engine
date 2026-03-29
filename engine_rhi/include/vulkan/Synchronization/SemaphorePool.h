//
// Created by C66 on 2026/3/29.
//
// Semaphore 对象池
//
#pragma once

#include "vulkan/Synchronization/Semaphore.h"
#include "vulkan/Synchronization/SyncConfig.h"
#include <vector>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    // Semaphore 对象池
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
            bool     IsTimeline() const { return config_.timeline; }

        private:
            bool CreateSemaphore(VkSemaphore& outSemaphore);

            DeviceManager&      deviceManager_;
            SemaphorePoolConfig config_;

            std::vector<VkSemaphore> availableSemaphores_;
            uint32_t                 activeCount_ = 0;
            uint32_t                 totalCount_  = 0;
    };
} // namespace engine::rhi::vulkan
