//
// Created by C66 on 2026/3/29.
//

#include "vulkan/Synchronization/SemaphorePool.h"
#include "vulkan/DeviceManager.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    SemaphorePool::SemaphorePool(DeviceManager& deviceManager) : deviceManager_(deviceManager) {}

    SemaphorePool::~SemaphorePool()
    {
        // 确保资源被清理
        if (totalCount_ > 0)
        {
            Shutdown();
        }
    }

    // ==================== 初始化/销毁 ====================

    bool SemaphorePool::Initialize(const SemaphorePoolConfig& config)
    {
        assert(totalCount_ == 0 && "SemaphorePool already initialized");

        config_ = config;

        // 预创建初始数量的 Semaphore
        availableSemaphores_.reserve(config_.initialCount);

        for (uint32_t i = 0; i < config_.initialCount; ++i)
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            if (!CreateSemaphore(semaphore))
            {
                // 创建失败，清理已创建的
                Shutdown();
                return false;
            }
            availableSemaphores_.push_back(semaphore);
        }

        totalCount_  = config_.initialCount;
        activeCount_ = 0;

        return true;
    }

    void SemaphorePool::Shutdown()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 销毁所有可用的 Semaphore
        for (VkSemaphore semaphore : availableSemaphores_)
        {
            if (semaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(device, semaphore, nullptr);
            }
        }
        availableSemaphores_.clear();

        // 注意：活跃的 Semaphore（已借出的）由 RAII 包装类管理
        // 这里我们重置计数，但不销毁活跃 Semaphore（它们会在 RAII 对象销毁时归还）
        totalCount_  = 0;
        activeCount_ = 0;
    }

    // ==================== 获取/归还 Semaphore ====================

    Semaphore SemaphorePool::Acquire()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 如果有可用的，直接返回
        if (!availableSemaphores_.empty())
        {
            VkSemaphore semaphore = availableSemaphores_.back();
            availableSemaphores_.pop_back();
            activeCount_++;

            // 创建归还回调
            auto deleter = [this](VkSemaphore s) { this->Release(s); };

            return Semaphore(device, semaphore, config_.timeline, deleter);
        }

        // 没有可用的，检查是否可以创建新的
        if (totalCount_ < config_.maxCount)
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            if (CreateSemaphore(semaphore))
            {
                totalCount_++;
                activeCount_++;

                auto deleter = [this](VkSemaphore s) { this->Release(s); };
                return Semaphore(device, semaphore, config_.timeline, deleter);
            }
        }

        // 无法获取 Semaphore
        return Semaphore();
    }

    void SemaphorePool::Release(VkSemaphore semaphore)
    {
        if (semaphore == VK_NULL_HANDLE)
            return;

        availableSemaphores_.push_back(semaphore);
        activeCount_--;
    }

    // ==================== 私有方法 ====================

    bool SemaphorePool::CreateSemaphore(VkSemaphore& outSemaphore)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        // 时间线信号量需要额外的结构体
        VkSemaphoreTypeCreateInfo timelineInfo = {};
        if (config_.timeline)
        {
            timelineInfo.sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineInfo.initialValue  = 0;

            semaphoreInfo.pNext = &timelineInfo;
        }

        VkResult result =
            vkCreateSemaphore(deviceManager_.GetLogicalDevice(), &semaphoreInfo, nullptr, &outSemaphore);

        return result == VK_SUCCESS;
    }

} // namespace engine::rhi::vulkan
