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

    // ==================== Timeline Semaphore ====================

    TimelineSemaphore SynchronizationManager::CreateTimelineSemaphore(uint64_t initialValue)
    {
        if (!timelineSupported_)
            return TimelineSemaphore();

        VkDevice device = deviceManager_.GetLogicalDevice();

        VkSemaphoreTypeCreateInfo timelineInfo = {};
        timelineInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineInfo.initialValue  = initialValue;

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext                 = &timelineInfo;

        VkSemaphore handle = VK_NULL_HANDLE;
        VkResult result    = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &handle);

        if (result != VK_SUCCESS)
            return TimelineSemaphore();

        return TimelineSemaphore(device, handle);
    }

} // namespace engine::rhi::vulkan
