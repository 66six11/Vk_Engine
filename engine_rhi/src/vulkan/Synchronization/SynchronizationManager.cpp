//
// Created by C66 on 2026/3/29.
//

#include "vulkan/Synchronization/SynchronizationManager.h"
#include "vulkan/DeviceManager.h"

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    SynchronizationManager::SynchronizationManager(DeviceManager& deviceManager)
        : deviceManager_(deviceManager)
    {
    }

    SynchronizationManager::~SynchronizationManager()
    {
        if (initialized_)
        {
            Shutdown();
        }
    }

    // ==================== 初始化/销毁 ====================

    bool SynchronizationManager::Initialize()
    {
        if (initialized_)
            return true;

        // 从 DeviceManager 查询时间线信号量支持（已在创建设备时检测）
        timelineSupported_ = deviceManager_.SupportsTimelineSemaphore();

        // 创建 Fence 池
        fencePool_ = std::make_unique<FencePool>(deviceManager_);
        if (!fencePool_->Initialize(fenceConfig_))
        {
            fencePool_.reset();
            return false;
        }

        // 创建二进制信号量池
        binarySemaphorePool_ = std::make_unique<SemaphorePool>(deviceManager_);
        if (!binarySemaphorePool_->Initialize(binarySemaphoreConfig_))
        {
            fencePool_->Shutdown();
            fencePool_.reset();
            return false;
        }

        // 创建时间线信号量池（如果支持）
        if (timelineSupported_)
        {
            // 设置时间线信号量配置
            timelineSemaphoreConfig_.timeline = true;

            timelineSemaphorePool_ = std::make_unique<SemaphorePool>(deviceManager_);
            if (!timelineSemaphorePool_->Initialize(timelineSemaphoreConfig_))
            {
                // 时间线信号量池创建失败不是致命错误，降级处理
                timelineSemaphorePool_.reset();
                timelineSupported_ = false;
            }
        }

        initialized_ = true;
        return true;
    }

    void SynchronizationManager::Shutdown()
    {
        if (!initialized_)
            return;

        // 先重置池，确保资源被正确归还
        if (fencePool_)
        {
            fencePool_->Shutdown();
            fencePool_.reset();
        }

        if (binarySemaphorePool_)
        {
            binarySemaphorePool_->Shutdown();
            binarySemaphorePool_.reset();
        }

        if (timelineSemaphorePool_)
        {
            timelineSemaphorePool_->Shutdown();
            timelineSemaphorePool_.reset();
        }

        initialized_       = false;
        timelineSupported_ = false;
    }

    // ==================== 批量获取 ====================

    std::vector<Fence> SynchronizationManager::AcquireFences(uint32_t count)
    {
        std::vector<Fence> fences;
        fences.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            Fence fence = AcquireFence();
            if (!fence)
            {
                // 获取失败，返回已获取的（调用者需要处理部分失败的情况）
                break;
            }
            fences.push_back(std::move(fence));
        }

        return fences;
    }

    std::vector<Semaphore> SynchronizationManager::AcquireBinarySemaphores(uint32_t count)
    {
        std::vector<Semaphore> semaphores;
        semaphores.reserve(count);

        for (uint32_t i = 0; i < count; ++i)
        {
            Semaphore semaphore = AcquireBinarySemaphore();
            if (!semaphore)
            {
                // 获取失败，返回已获取的
                break;
            }
            semaphores.push_back(std::move(semaphore));
        }

        return semaphores;
    }

} // namespace engine::rhi::vulkan
