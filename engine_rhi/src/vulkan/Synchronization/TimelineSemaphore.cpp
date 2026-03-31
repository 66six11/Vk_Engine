//
// Created by C66 on 2026/3/31.
//

#include "vulkan/Synchronization/TimelineSemaphore.h"
#include <cassert>
#include <stdexcept>
#include <string>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    TimelineSemaphore::TimelineSemaphore(VkDevice device, VkSemaphore handle, uint64_t initialValue)
        : device_(device)
        , handle_(handle)
        , currentValue_(initialValue)
    {
        assert(device_ != VK_NULL_HANDLE);
        assert(handle_ != VK_NULL_HANDLE);
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        Destroy();
    }

    // ==================== 移动语义 ====================

    TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& other) noexcept
        : device_(other.device_)
        , handle_(other.handle_)
        , currentValue_(other.currentValue_.load())
    {
        other.device_ = VK_NULL_HANDLE;
        other.handle_ = VK_NULL_HANDLE;
        other.currentValue_ = 0;
    }

    TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            device_ = other.device_;
            handle_ = other.handle_;
            currentValue_ = other.currentValue_.load();

            other.device_ = VK_NULL_HANDLE;
            other.handle_ = VK_NULL_HANDLE;
            other.currentValue_ = 0;
        }
        return *this;
    }

  

    void TimelineSemaphore::WaitLatest(uint64_t timeoutNs) const
    {
        if (handle_ == VK_NULL_HANDLE)
            return;

        uint64_t latestValue = currentValue_.load();

        // 如果值为0，说明还没有 Signal 过，无需等待
        if (latestValue == 0)
            return;

        Wait(latestValue, timeoutNs);
    }

    // ==================== 查询 ====================

    uint64_t TimelineSemaphore::GetCounterValue() const
    {
        if (handle_ == VK_NULL_HANDLE || device_ == VK_NULL_HANDLE)
            return 0;

        uint64_t value = 0;
        vkGetSemaphoreCounterValue(device_, handle_, &value);
        return value;
    }

    // ==================== 私有方法 ====================

    void TimelineSemaphore::Wait(uint64_t value, uint64_t timeoutNs) const
    {
        if (handle_ == VK_NULL_HANDLE || device_ == VK_NULL_HANDLE)
            return;

        VkSemaphoreWaitInfo waitInfo = {};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.flags = 0;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &handle_;
        waitInfo.pValues = &value;

        vkWaitSemaphores(device_, &waitInfo, timeoutNs);
    }

    // ==================== 内部方法 ====================

    void TimelineSemaphore::Destroy()
    {
        if (handle_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE)
        {
            // 等待所有挂起的操作完成
            WaitLatest();
            vkDestroySemaphore(device_, handle_, nullptr);
        }
        handle_ = VK_NULL_HANDLE;
        device_ = VK_NULL_HANDLE;
        currentValue_ = 0;
    }

} // namespace engine::rhi::vulkan