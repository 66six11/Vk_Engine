//
// Created by C66 on 2026/3/29.
//
// SynchronizationManager - 同步管理器聚合头文件
// 提供统一的同步原语管理接口
//
#pragma once

// 包含所有同步相关子模块
#include "vulkan/Synchronization/Fence.h"
#include "vulkan/Synchronization/FencePool.h"
#include "vulkan/Synchronization/Semaphore.h"
#include "vulkan/Synchronization/SemaphorePool.h"
#include "vulkan/Synchronization/TimelineSemaphore.h"
#include "vulkan/Synchronization/SyncConfig.h"

#include <memory>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    // ==================== 统一管理器 ====================

    // 同步原语管理器
    class SynchronizationManager
    {
        public:
            explicit SynchronizationManager(DeviceManager& deviceManager);
            ~SynchronizationManager();

            // 禁止拷贝/移动
            SynchronizationManager(const SynchronizationManager&)            = delete;
            SynchronizationManager& operator=(const SynchronizationManager&) = delete;
            SynchronizationManager(SynchronizationManager&&)                 = delete;
            SynchronizationManager& operator=(SynchronizationManager&&)      = delete;

            // 初始化/销毁
            bool Initialize();
            void Shutdown();

            // 配置（必须在 Initialize 前设置）
            void SetFenceConfig(const FencePoolConfig& config) { fenceConfig_ = config; }
            void SetBinarySemaphoreConfig(const SemaphorePoolConfig& config) { binarySemaphoreConfig_ = config; }

            // 获取各类池
            FencePool&     GetFencePool() { return *fencePool_; }
            SemaphorePool& GetBinarySemaphorePool() { return *binarySemaphorePool_; }

            // 便捷方法：直接获取同步对象
            Fence     AcquireFence() { return fencePool_->Acquire(); }
            Semaphore AcquireBinarySemaphore() { return binarySemaphorePool_->Acquire(); }

            // Timeline Semaphore 管理（长期持有，不池化）
            TimelineSemaphore CreateTimelineSemaphore(uint64_t initialValue = 0);

            // 批量获取
            std::vector<Fence>     AcquireFences(uint32_t count);
            std::vector<Semaphore> AcquireBinarySemaphores(uint32_t count);

            // 检查时间线信号量支持
            bool IsTimelineSemaphoreSupported() const { return timelineSupported_; }

            // 工具方法
            void WaitForAllFences() { fencePool_->WaitForAll(); }
            void ResetAllFences() { fencePool_->ResetAll(); }

        private:
            DeviceManager& deviceManager_;

            std::unique_ptr<FencePool>     fencePool_;
            std::unique_ptr<SemaphorePool> binarySemaphorePool_;

            FencePoolConfig     fenceConfig_;
            SemaphorePoolConfig binarySemaphoreConfig_;

            bool initialized_       = false;
            bool timelineSupported_ = false;
    };
} // namespace engine::rhi::vulkan
