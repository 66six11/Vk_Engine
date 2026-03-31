//
// Created by C66 on 2026/3/31.
//
// CommandQueue - Vulkan Queue 提交实现
//

#include "vulkan/Command/CommandQueue.h"
#include "vulkan/Command/CommandBuffer.h"
#include "vulkan/Synchronization/Fence.h"
#include "vulkan/Synchronization/Semaphore.h"
#include "vulkan/Synchronization/TimelineSemaphore.h"
#include "vulkan/SwapChainManager.h"

namespace engine::rhi::vulkan
{
    // ==================== 构造函数/析构函数 ====================

    CommandQueue::CommandQueue() = default;

    CommandQueue::CommandQueue(VkQueue handle, uint32_t familyIndex, uint32_t queueIndex)
        : handle_(handle), familyIndex_(familyIndex), queueIndex_(queueIndex)
    {
    }

    // ==================== 移动语义 ====================

    CommandQueue::CommandQueue(CommandQueue&& other) noexcept
        : handle_(other.handle_), familyIndex_(other.familyIndex_), queueIndex_(other.queueIndex_)
    {
        other.handle_ = VK_NULL_HANDLE;
        other.familyIndex_ = UINT32_MAX;
        other.queueIndex_ = 0;
    }

    CommandQueue& CommandQueue::operator=(CommandQueue&& other) noexcept
    {
        if (this != &other)
        {
            handle_ = other.handle_;
            familyIndex_ = other.familyIndex_;
            queueIndex_ = other.queueIndex_;

            other.handle_ = VK_NULL_HANDLE;
            other.familyIndex_ = UINT32_MAX;
            other.queueIndex_ = 0;
        }
        return *this;
    }

    // ==================== 提交方法 ====================

    void CommandQueue::Submit(CommandBuffer& cmd, Fence* signalFence)
    {
        if (handle_ == VK_NULL_HANDLE) return;

        VkCommandBuffer cmdHandle = cmd.GetHandle();

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdHandle;

        VkFence fenceHandle = signalFence ? signalFence->GetHandle() : VK_NULL_HANDLE;

        vkQueueSubmit(handle_, 1, &submit, fenceHandle);
    }

    void CommandQueue::Submit(
        CommandBuffer& cmd,
        Semaphore* waitSemaphore,
        Semaphore* signalSemaphore,
        Fence* signalFence,
        VkPipelineStageFlags waitStage)
    {
        if (handle_ == VK_NULL_HANDLE) return;

        VkCommandBuffer cmdHandle = cmd.GetHandle();

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // 等待信号量
        VkSemaphore waitSem = waitSemaphore ? waitSemaphore->GetHandle() : VK_NULL_HANDLE;
        if (waitSem != VK_NULL_HANDLE)
        {
            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &waitSem;
            submit.pWaitDstStageMask = &waitStage;
        }

        // 命令缓冲
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdHandle;

        // 信号信号量
        VkSemaphore signalSem = signalSemaphore ? signalSemaphore->GetHandle() : VK_NULL_HANDLE;
        if (signalSem != VK_NULL_HANDLE)
        {
            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &signalSem;
        }

        // Fence
        VkFence fenceHandle = signalFence ? signalFence->GetHandle() : VK_NULL_HANDLE;

        vkQueueSubmit(handle_, 1, &submit, fenceHandle);
    }

    // ==================== Timeline Semaphore 顺序提交（自动 wait/signal）====================

    void CommandQueue::Submit(CommandBuffer& cmd, TimelineSemaphore* timeline, Fence* fence)
    {
        if (handle_ == VK_NULL_HANDLE || !timeline) return;

        // 获取 wait/signal 值
        uint64_t waitValue = timeline->GetCurrentValue();      // 等待已完成的最新的
        uint64_t signalValue = timeline->GetNextSignalValue(); // signal 下一个

        // 如果是第一次提交（current = 0），不需要 wait
        bool hasWait = (waitValue > 0);

        VkCommandBuffer cmdHandle = cmd.GetHandle();
        VkSemaphore semHandle = timeline->GetHandle();

        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {};
        timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        if (hasWait) {
            timelineSubmitInfo.waitSemaphoreValueCount = 1;
            timelineSubmitInfo.pWaitSemaphoreValues = &waitValue;
        }
        timelineSubmitInfo.signalSemaphoreValueCount = 1;
        timelineSubmitInfo.pSignalSemaphoreValues = &signalValue;

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pNext = &timelineSubmitInfo;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdHandle;
        if (hasWait) {
            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &semHandle;
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            submit.pWaitDstStageMask = &waitStage;
        }
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &semHandle;

        VkFence fenceHandle = fence ? fence->GetHandle() : VK_NULL_HANDLE;

        if (vkQueueSubmit(handle_, 1, &submit, fenceHandle) == VK_SUCCESS) {
            // 提交成功后，更新 Timeline 内部值
            timeline->SignalInternal(); // 实际 signal 值已在 GPU 端
        }
    }

    // ==================== Timeline Semaphore 显式值提交（RenderGraph）====================

    void CommandQueue::Submit(
        CommandBuffer& cmd,
        TimelineSemaphore* timeline,
        uint64_t waitValue,
        uint64_t signalValue,
        Fence* fence)
    {
        if (handle_ == VK_NULL_HANDLE || !timeline) return;

        VkCommandBuffer cmdHandle = cmd.GetHandle();
        VkSemaphore semHandle = timeline->GetHandle();

        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {};
        timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineSubmitInfo.waitSemaphoreValueCount = 1;
        timelineSubmitInfo.pWaitSemaphoreValues = &waitValue;
        timelineSubmitInfo.signalSemaphoreValueCount = 1;
        timelineSubmitInfo.pSignalSemaphoreValues = &signalValue;

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pNext = &timelineSubmitInfo;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdHandle;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &semHandle;
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        submit.pWaitDstStageMask = &waitStage;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &semHandle;

        VkFence fenceHandle = fence ? fence->GetHandle() : VK_NULL_HANDLE;

        vkQueueSubmit(handle_, 1, &submit, fenceHandle);
        // 注意：显式提交不更新内部值，由调用者管理
    }

    // ==================== Present ====================

    bool CommandQueue::Present(SwapChainManager& swapChain, uint32_t imageIndex, Semaphore* renderCompleteSemaphore)
    {
        if (handle_ == VK_NULL_HANDLE) return false;

        VkSemaphore signalSem = renderCompleteSemaphore ? renderCompleteSemaphore->GetHandle() : VK_NULL_HANDLE;

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = signalSem != VK_NULL_HANDLE ? 1 : 0;
        presentInfo.pWaitSemaphores = signalSem != VK_NULL_HANDLE ? &signalSem : nullptr;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChainHandle = swapChain.GetSwapChain();
        presentInfo.pSwapchains = &swapChainHandle;
        presentInfo.pImageIndices = &imageIndex;

        VkResult result = vkQueuePresentKHR(handle_, &presentInfo);

        // 处理需要重建交换链的情况
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            return false;
        }

        return result == VK_SUCCESS;
    }

    // ==================== 等待 ====================

    void CommandQueue::WaitIdle() const
    {
        if (handle_ != VK_NULL_HANDLE)
        {
            vkQueueWaitIdle(handle_);
        }
    }

} // namespace engine::rhi::vulkan
