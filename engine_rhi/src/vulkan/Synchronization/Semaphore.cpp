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
        std::function<void(VkSemaphore)> deleter)
        : device_(device), handle_(handle), deleter_(std::move(deleter))
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
          deleter_(std::move(other.deleter_))
    {
        other.device_ = VK_NULL_HANDLE;
        other.handle_ = VK_NULL_HANDLE;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            device_ = other.device_;
            handle_ = other.handle_;
            deleter_ = std::move(other.deleter_);

            other.device_ = VK_NULL_HANDLE;
            other.handle_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    // ==================== 内部方法 ====================

    VkSemaphore Semaphore::Release()
    {
        VkSemaphore releasedHandle = handle_;
        handle_                    = VK_NULL_HANDLE;
        device_                    = VK_NULL_HANDLE;
        deleter_                   = nullptr;
        return releasedHandle;
    }

    void Semaphore::Destroy()
    {
        if (handle_ != VK_NULL_HANDLE && deleter_)
        {
            deleter_(handle_);
        }
        handle_  = VK_NULL_HANDLE;
        device_  = VK_NULL_HANDLE;
        deleter_ = nullptr;
    }

} // namespace engine::rhi::vulkan
