//
// Created by C66 on 2026/3/29.
//

#include "vulkan/Synchronization/Semaphore.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    Semaphore::Semaphore(
        VkDevice                         device,
        VkSemaphore                      handle,
        bool                             isTimeline,
        std::function<void(VkSemaphore)> deleter)
        : device_(device), handle_(handle), isTimeline_(isTimeline), deleter_(std::move(deleter))
    {
        assert(device_ != VK_NULL_HANDLE);
        assert(handle_ != VK_NULL_HANDLE);
    }

    Semaphore::~Semaphore()
    {
        Destroy();
    }

    // ==================== 移动语义 ====================

    Semaphore::Semaphore(Semaphore&& other) noexcept
        : device_(other.device_),
          handle_(other.handle_),
          isTimeline_(other.isTimeline_),
          deleter_(std::move(other.deleter_))
    {
        other.device_     = VK_NULL_HANDLE;
        other.handle_     = VK_NULL_HANDLE;
        other.isTimeline_ = false;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            device_     = other.device_;
            handle_     = other.handle_;
            isTimeline_ = other.isTimeline_;
            deleter_    = std::move(other.deleter_);

            other.device_     = VK_NULL_HANDLE;
            other.handle_     = VK_NULL_HANDLE;
            other.isTimeline_ = false;
        }
        return *this;
    }

    // ==================== 时间线信号量接口 ====================

    uint64_t Semaphore::GetCounterValue() const
    {
        if (!isTimeline_ || handle_ == VK_NULL_HANDLE)
            return 0;

        uint64_t value = 0;
        vkGetSemaphoreCounterValue(device_, handle_, &value);
        return value;
    }

    void Semaphore::Signal(uint64_t value) const
    {
        if (!isTimeline_ || handle_ == VK_NULL_HANDLE)
            return;

        VkSemaphoreSignalInfo signalInfo = {};
        signalInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.semaphore             = handle_;
        signalInfo.value                 = value;

        vkSignalSemaphore(device_, &signalInfo);
    }

    void Semaphore::Wait(uint64_t value, uint64_t timeoutNs) const
    {
        if (!isTimeline_ || handle_ == VK_NULL_HANDLE)
            return;

        VkSemaphoreWaitInfo waitInfo = {};
        waitInfo.sType               = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.flags               = 0;
        waitInfo.semaphoreCount      = 1;
        waitInfo.pSemaphores         = &handle_;
        waitInfo.pValues             = &value;

        vkWaitSemaphores(device_, &waitInfo, timeoutNs);
    }

    // ==================== 内部方法 ====================

    VkSemaphore Semaphore::Release()
    {
        VkSemaphore releasedHandle = handle_;
        handle_                    = VK_NULL_HANDLE;
        device_                    = VK_NULL_HANDLE;
        isTimeline_                = false;
        deleter_                   = nullptr;
        return releasedHandle;
    }

    void Semaphore::Destroy()
    {
        if (handle_ != VK_NULL_HANDLE && deleter_)
        {
            deleter_(handle_);
        }
        handle_     = VK_NULL_HANDLE;
        device_     = VK_NULL_HANDLE;
        isTimeline_ = false;
        deleter_    = nullptr;
    }

} // namespace engine::rhi::vulkan
