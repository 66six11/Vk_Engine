//
// Created by C66 on 2026/3/31.
//

#include "vulkan/Command/CommandBuffer.h"
#include "vulkan/Command/CommandPool.h"
#include <cassert>

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    CommandBuffer::CommandBuffer()
        : handle_(VK_NULL_HANDLE)
        , pool_(nullptr)
        , isRecording_(false)
    {
    }

    CommandBuffer::CommandBuffer(VkCommandBuffer handle, CommandPool* pool)
        : handle_(handle)
        , pool_(pool)
        , isRecording_(false)
    {
        assert(handle_ != VK_NULL_HANDLE);
    }

    CommandBuffer::~CommandBuffer()
    {
        Reset();
    }

    // ==================== 移动语义 ====================

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
        : handle_(other.handle_)
        , pool_(other.pool_)
        , isRecording_(other.isRecording_)
    {
        other.handle_ = VK_NULL_HANDLE;
        other.pool_ = nullptr;
        other.isRecording_ = false;
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Reset();

            handle_ = other.handle_;
            pool_ = other.pool_;
            isRecording_ = other.isRecording_;

            other.handle_ = VK_NULL_HANDLE;
            other.pool_ = nullptr;
            other.isRecording_ = false;
        }
        return *this;
    }

    // ==================== 录制控制 ====================

    bool CommandBuffer::Begin(VkCommandBufferUsageFlags usage)
    {
        if (handle_ == VK_NULL_HANDLE || isRecording_)
            return false;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = usage;

        VkResult result = vkBeginCommandBuffer(handle_, &beginInfo);
        if (result != VK_SUCCESS)
            return false;

        isRecording_ = true;
        return true;
    }

    bool CommandBuffer::End()
    {
        if (handle_ == VK_NULL_HANDLE || !isRecording_)
            return false;

        VkResult result = vkEndCommandBuffer(handle_);
        if (result != VK_SUCCESS)
            return false;

        isRecording_ = false;
        return true;
    }

    // ==================== 内部方法 ====================

    void CommandBuffer::Reset()
    {
        if (handle_ != VK_NULL_HANDLE && pool_)
        {
            // 如果在录制中，先结束录制
            if (isRecording_)
            {
                vkEndCommandBuffer(handle_);
                isRecording_ = false;
            }

            // 归还到 Pool
            pool_->Free(handle_, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        }

        handle_ = VK_NULL_HANDLE;
        pool_ = nullptr;
        isRecording_ = false;
    }

} // namespace engine::rhi::vulkan
