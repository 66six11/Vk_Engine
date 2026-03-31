//
// Created by C66 on 2026/3/31.
//

#include "vulkan/Command/CommandPool.h"
#include "vulkan/DeviceManager.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    CommandPool::CommandPool(DeviceManager& deviceManager, uint32_t queueFamilyIndex)
        : deviceManager_(deviceManager)
        , queueFamilyIndex_(queueFamilyIndex)
        , pool_(VK_NULL_HANDLE)
    {
    }

    CommandPool::~CommandPool()
    {
        if (pool_ != VK_NULL_HANDLE)
        {
            Shutdown();
        }
    }

    // ==================== 初始化/销毁 ====================

    bool CommandPool::Initialize(const CommandPoolConfig& config)
    {
        assert(pool_ == VK_NULL_HANDLE && "CommandPool already initialized");

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex_;
        poolInfo.flags = config.flags;

        VkResult result = vkCreateCommandPool(
            deviceManager_.GetLogicalDevice(),
            &poolInfo,
            nullptr,
            &pool_);

        if (result != VK_SUCCESS)
            return false;

        // 预分配初始数量的 CommandBuffer
        if (config.initialBufferCount > 0)
        {
            if (!AllocateFromPool(config.initialBufferCount, VK_COMMAND_BUFFER_LEVEL_PRIMARY))
            {
                Shutdown();
                return false;
            }
        }

        return true;
    }

    void CommandPool::Shutdown()
    {
        if (pool_ == VK_NULL_HANDLE)
            return;

        // 清空空闲列表
        {
            std::lock_guard<std::mutex> lock(mutex_);
            freePrimaryBuffers_.clear();
            freeSecondaryBuffers_.clear();
        }

        vkDestroyCommandPool(deviceManager_.GetLogicalDevice(), pool_, nullptr);
        pool_ = VK_NULL_HANDLE;
    }

    // ==================== 分配/归还 ====================

    CommandBuffer CommandPool::Allocate(VkCommandBufferLevel level)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        VkCommandBuffer buffer = VK_NULL_HANDLE;

        // 从空闲列表获取
        if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY && !freePrimaryBuffers_.empty())
        {
            buffer = freePrimaryBuffers_.back();
            freePrimaryBuffers_.pop_back();
        }
        else if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY && !freeSecondaryBuffers_.empty())
        {
            buffer = freeSecondaryBuffers_.back();
            freeSecondaryBuffers_.pop_back();
        }
        else
        {
            // 空闲列表为空，分配新的
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = pool_;
            allocInfo.level = level;
            allocInfo.commandBufferCount = 1;

            VkResult result = vkAllocateCommandBuffers(
                deviceManager_.GetLogicalDevice(),
                &allocInfo,
                &buffer);

            if (result != VK_SUCCESS)
                return CommandBuffer();
        }

        return CommandBuffer(buffer, this);
    }

    void CommandPool::Free(VkCommandBuffer buffer, VkCommandBufferLevel level)
    {
        if (buffer == VK_NULL_HANDLE)
            return;

        std::lock_guard<std::mutex> lock(mutex_);

        if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            freePrimaryBuffers_.push_back(buffer);
        }
        else
        {
            freeSecondaryBuffers_.push_back(buffer);
        }
    }

    // ==================== 重置 ====================

    void CommandPool::Reset()
    {
        if (pool_ == VK_NULL_HANDLE)
            return;

        // 重置整个 Pool，所有 CommandBuffer 回到初始状态
        vkResetCommandPool(deviceManager_.GetLogicalDevice(), pool_, 0);

        // 清空空闲列表（因为 Pool 已重置，所有 Buffer 都失效了）
        std::lock_guard<std::mutex> lock(mutex_);
        freePrimaryBuffers_.clear();
        freeSecondaryBuffers_.clear();
    }

    // ==================== 私有方法 ====================

    bool CommandPool::AllocateFromPool(uint32_t count, VkCommandBufferLevel level)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool_;
        allocInfo.level = level;
        allocInfo.commandBufferCount = count;

        std::vector<VkCommandBuffer> buffers(count);
        VkResult result = vkAllocateCommandBuffers(
            deviceManager_.GetLogicalDevice(),
            &allocInfo,
            buffers.data());

        if (result != VK_SUCCESS)
            return false;

        // 加入空闲列表
        if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            freePrimaryBuffers_.insert(
                freePrimaryBuffers_.end(),
                buffers.begin(),
                buffers.end());
        }
        else
        {
            freeSecondaryBuffers_.insert(
                freeSecondaryBuffers_.end(),
                buffers.begin(),
                buffers.end());
        }

        return true;
    }

} // namespace engine::rhi::vulkan
