//
// Created by C66 on 2026/3/31.
//
// CommandQueue - Vulkan Queue 封装
// 职责：提交 CommandBuffer 到 GPU，处理 Present
// 线程安全：单个 Queue 同一时间只能一个线程提交
//
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace engine::rhi::vulkan
{
    // 前置声明
    class CommandBuffer;
    class Fence;
    class Semaphore;
    class TimelineSemaphore;
    class SwapChainManager;



    // ==================== CommandQueue ====================

    class CommandQueue
    {
    public:
        CommandQueue();
        CommandQueue(VkQueue handle, uint32_t familyIndex, uint32_t queueIndex);
        ~CommandQueue() = default;

        // 禁止拷贝
        CommandQueue(const CommandQueue&) = delete;
        CommandQueue& operator=(const CommandQueue&) = delete;

        // 允许移动
        CommandQueue(CommandQueue&& other) noexcept;
        CommandQueue& operator=(CommandQueue&& other) noexcept;

        // ==================== 基础提交 ====================
        // 单个提交（无信号量）
        void Submit(CommandBuffer& cmd, Fence* fence = nullptr);

        // ==================== Binary Semaphore 提交（SwapChain）====================
        void Submit(
            CommandBuffer& cmd,
            Semaphore* waitSemaphore,
            Semaphore* signalSemaphore,
            Fence* fence = nullptr,
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // ==================== Timeline Semaphore 顺序提交（推荐）====================
        // 自动等待当前值，signal 下一个值
        // 适用于：单队列顺序渲染，按调用顺序自动同步
        void Submit(CommandBuffer& cmd, TimelineSemaphore* timeline, Fence* fence = nullptr);

        // ==================== Timeline Semaphore 显式值提交（RenderGraph）====================
        // 显式指定 wait/signal 值
        // 适用于：多队列、复杂依赖、需要精确控制
        void Submit(
            CommandBuffer& cmd,
            TimelineSemaphore* timeline,
            uint64_t waitValue,
            uint64_t signalValue,
            Fence* fence = nullptr);

        // Present 到交换链
        // 返回是否成功，需要重建交换链时返回 false
        bool Present(SwapChainManager& swapChain, uint32_t imageIndex, Semaphore* renderCompleteSemaphore);

        // 等待队列空闲（阻塞）
        void WaitIdle() const;

        // 访问器
        VkQueue GetHandle() const { return handle_; }
        uint32_t GetFamilyIndex() const { return familyIndex_; }
        uint32_t GetQueueIndex() const { return queueIndex_; }
        bool IsValid() const { return handle_ != VK_NULL_HANDLE; }

    private:
        VkQueue handle_ = VK_NULL_HANDLE;
        uint32_t familyIndex_ = UINT32_MAX;
        uint32_t queueIndex_ = 0;
    };

} // namespace engine::rhi::vulkan
