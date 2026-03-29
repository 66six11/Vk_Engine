//
// Created by C66 on 2026/3/29.
//

#include "vulkan/Synchronization/FencePool.h"
#include "vulkan/DeviceManager.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    FencePool::FencePool(DeviceManager& deviceManager) : deviceManager_(deviceManager) {}

    FencePool::~FencePool()
    {
        // 确保资源被清理
        if (totalCount_ > 0)
        {
            Shutdown();
        }
    }

    // ==================== 初始化/销毁 ====================

    bool FencePool::Initialize(const FencePoolConfig& config)
    {
        assert(totalCount_ == 0 && "FencePool already initialized");

        config_ = config;

        // 预创建初始数量的 Fence
        availableFences_.reserve(config_.initialCount);

        for (uint32_t i = 0; i < config_.initialCount; ++i)
        {
            VkFence fence = VK_NULL_HANDLE;
            if (!CreateFence(fence))
            {
                // 创建失败，清理已创建的
                Shutdown();
                return false;
            }
            availableFences_.push_back(fence);
        }

        totalCount_  = config_.initialCount;
        activeCount_ = 0;

        return true;
    }

    void FencePool::Shutdown()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 销毁所有可用的 Fence
        for (VkFence fence : availableFences_)
        {
            if (fence != VK_NULL_HANDLE)
            {
                vkDestroyFence(device, fence, nullptr);
            }
        }
        availableFences_.clear();

        // 注意：活跃的 Fence（已借出的）由 RAII 包装类管理
        // 这里我们重置计数，但不销毁活跃 Fence（它们会在 RAII 对象销毁时归还）
        totalCount_  = 0;
        activeCount_ = 0;
    }

    // ==================== 获取/归还 Fence ====================

    Fence FencePool::Acquire()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 如果有可用的，直接返回
        if (!availableFences_.empty())
        {
            VkFence fence = availableFences_.back();
            availableFences_.pop_back();
            activeCount_++;

            // 创建归还回调
            auto deleter = [this](VkFence f) { this->Release(f); };

            return Fence(device, fence, deleter);
        }

        // 没有可用的，检查是否可以创建新的
        if (totalCount_ < config_.maxCount)
        {
            VkFence fence = VK_NULL_HANDLE;
            if (CreateFence(fence))
            {
                totalCount_++;
                activeCount_++;

                auto deleter = [this](VkFence f) { this->Release(f); };
                return Fence(device, fence, deleter);
            }
        }

        // 无法获取 Fence
        return Fence();
    }

    void FencePool::Release(VkFence fence)
    {
        if (fence == VK_NULL_HANDLE)
            return;

        // 重置 Fence 状态以便复用
        vkResetFences(deviceManager_.GetLogicalDevice(), 1, &fence);

        availableFences_.push_back(fence);
        activeCount_--;
    }

    // ==================== 等待/重置 ====================

    void FencePool::WaitForAll(uint64_t timeoutNs)
    {
        // 注意：这里只等待池中的可用 Fence（它们应该是已信号的）
        // 活跃的 Fence 由调用者自己管理
        if (availableFences_.empty())
            return;

        vkWaitForFences(
            deviceManager_.GetLogicalDevice(),
            static_cast<uint32_t>(availableFences_.size()),
            availableFences_.data(),
            VK_TRUE, // 等待所有
            timeoutNs);
    }

    void FencePool::ResetAll()
    {
        if (availableFences_.empty())
            return;

        vkResetFences(
            deviceManager_.GetLogicalDevice(),
            static_cast<uint32_t>(availableFences_.size()),
            availableFences_.data());
    }

    // ==================== 私有方法 ====================

    bool FencePool::CreateFence(VkFence& outFence)
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = config_.createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        VkResult result =
            vkCreateFence(deviceManager_.GetLogicalDevice(), &fenceInfo, nullptr, &outFence);

        return result == VK_SUCCESS;
    }

} // namespace engine::rhi::vulkan
