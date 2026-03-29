//
// Created by C66 on 2026/3/29.
//

#include "vulkan/Synchronization/Fence.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    Fence::Fence(VkDevice device, VkFence handle, std::function<void(VkFence)> deleter)
        : device_(device), handle_(handle), deleter_(std::move(deleter))
    {
        assert(device_ != VK_NULL_HANDLE);
        assert(handle_ != VK_NULL_HANDLE);
    }

    Fence::~Fence()
    {
        Destroy();
    }

    // ==================== 移动语义 ====================

    Fence::Fence(Fence&& other) noexcept
        : device_(other.device_), handle_(other.handle_), deleter_(std::move(other.deleter_))
    {
        other.device_ = VK_NULL_HANDLE;
        other.handle_ = VK_NULL_HANDLE;
    }

    Fence& Fence::operator=(Fence&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            device_  = other.device_;
            handle_  = other.handle_;
            deleter_ = std::move(other.deleter_);

            other.device_ = VK_NULL_HANDLE;
            other.handle_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    // ==================== 状态查询 ====================

    bool Fence::IsSignaled() const
    {
        if (handle_ == VK_NULL_HANDLE)
            return false;

        VkResult result = vkGetFenceStatus(device_, handle_);
        return result == VK_SUCCESS;
    }

    bool Fence::Wait(uint64_t timeoutNs) const
    {
        if (handle_ == VK_NULL_HANDLE)
            return false;

        VkResult result = vkWaitForFences(device_, 1, &handle_, VK_TRUE, timeoutNs);
        return result == VK_SUCCESS;
    }

    bool Fence::WaitFor(std::chrono::milliseconds timeout) const
    {
        // 将毫秒转换为纳秒
        uint64_t timeoutNs = static_cast<uint64_t>(timeout.count()) * 1'000'000;
        return Wait(timeoutNs);
    }

    void Fence::Reset() const
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkResetFences(device_, 1, &handle_);
        }
    }

    // ==================== 内部方法 ====================

    VkFence Fence::Release()
    {
        VkFence releasedHandle = handle_;
        handle_                = VK_NULL_HANDLE;
        device_                = VK_NULL_HANDLE;
        deleter_               = nullptr;
        return releasedHandle;
    }

    void Fence::Destroy()
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