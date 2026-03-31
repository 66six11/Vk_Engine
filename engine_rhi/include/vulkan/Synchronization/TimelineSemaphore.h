//
// Created by C66 on 2026/3/31.
//
// Timeline Semaphore RAII 类 - 长期持有，不池化
// 支持自动值管理（内部计数器）或手动指定值
//
#pragma once

#include <vulkan/vulkan.h>
#include <atomic>
#include <cstdint>

namespace engine::rhi::vulkan
{
    // 时间线信号量 RAII 包装类
    class TimelineSemaphore
    {
        private:
            void Destroy();

            VkDevice                      device_ = VK_NULL_HANDLE;
            VkSemaphore                   handle_ = VK_NULL_HANDLE;
            mutable std::atomic<uint64_t> currentValue_{0};

        public:
            TimelineSemaphore() = default;
            TimelineSemaphore(VkDevice device, VkSemaphore handle, uint64_t initialValue = 0);
            ~TimelineSemaphore();

            // 禁止拷贝
            TimelineSemaphore(const TimelineSemaphore&)            = delete;
            TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;

            // 允许移动
            TimelineSemaphore(TimelineSemaphore&& other) noexcept;
            TimelineSemaphore& operator=(TimelineSemaphore&& other) noexcept;

            // ========== 模式1：顺序提交自动同步（推荐简单场景）==========
            // 适用于：单队列顺序渲染，按调用顺序自动同步
            // 行为类似 Binary Semaphore，但支持 wait-before-signal
            //
            // 使用方式：
            //   queue.Submit(cmd1, &timeline);  // 自动 signal 1
            //   queue.Submit(cmd2, &timeline);  // 自动 wait 1, signal 2
            //   queue.Submit(cmd3, &timeline);  // 自动 wait 2, signal 3
            //
            // 获取下一个要 signal 的值（current + 1），不自动递增
            uint64_t GetNextSignalValue() const { return currentValue_.load() + 1; }

            // 获取当前已 signal 的值（用于 wait）
            uint64_t GetCurrentValue() const { return currentValue_.load(); }

            // 等待最后一个提交完成（CPU 端等待）
            void WaitLatest(uint64_t timeoutNs = UINT64_MAX) const;

            // 内部使用：提交成功后更新内部计数器（顺序提交模式）
            void SignalInternal() { ++currentValue_; }

            // ========== 模式2：外部控制（RenderGraph 精细控制）==========
            // 适用于：多队列、复杂依赖、需要精确控制值
            // 此时外部管理值，TimelineSemaphore 只包装 VkSemaphore

            // 查询 GPU 当前实际值（调试用，会查询 GPU 状态）
            uint64_t GetCounterValue() const;

            // ========== 访问器 ==========

            VkSemaphore GetHandle() const { return handle_; }
            explicit    operator bool() const { return handle_ != VK_NULL_HANDLE; }

        private:
            // 以下方法仅供内部使用（如析构时等待）
            // 手动调用 Signal/Wait 会导致：
            // 1. 缓存不一致（本地值与 GPU 值不同步）
            // 2. 性能问题（Wait 会阻塞 CPU）
            // 3. 逻辑错误（跳过正常提交流程）
            
            // 内部：等待指定值（析构时使用，会阻塞 CPU）
            void Wait(uint64_t value, uint64_t timeoutNs = UINT64_MAX) const;
    };
} // namespace engine::rhi::vulkan
