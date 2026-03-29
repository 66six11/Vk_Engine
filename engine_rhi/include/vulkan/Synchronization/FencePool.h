//
// Created by C66 on 2026/3/29.
//
// Fence 对象池
//
#pragma once

#include "vulkan/Synchronization/Fence.h"
#include "vulkan/Synchronization/SyncConfig.h"
#include <vector>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    // Fence 对象池
    class FencePool
    {
        public:
            explicit FencePool(DeviceManager& deviceManager);
            ~FencePool();

            // 禁止拷贝/移动
            FencePool(const FencePool&)            = delete;
            FencePool& operator=(const FencePool&) = delete;
            FencePool(FencePool&&)                 = delete;
            FencePool& operator=(FencePool&&)      = delete;

            // 初始化/销毁
            bool Initialize(const FencePoolConfig& config);
            void Shutdown();

            // 获取/归还 Fence
            Fence Acquire();
            void  Release(VkFence fence);

            // 等待池中所有 Fence
            void WaitForAll(uint64_t timeoutNs = UINT64_MAX);
            void ResetAll();

            // 统计
            uint32_t GetAvailableCount() const { return static_cast<uint32_t>(availableFences_.size()); }
            uint32_t GetActiveCount() const { return activeCount_; }
            uint32_t GetTotalCount() const { return totalCount_; }

        private:
            bool CreateFence(VkFence& outFence);

            DeviceManager&  deviceManager_;
            FencePoolConfig config_;

            std::vector<VkFence> availableFences_; // 可用列表
            uint32_t             activeCount_ = 0;
            uint32_t             totalCount_  = 0;
    };
} // namespace engine::rhi::vulkan
