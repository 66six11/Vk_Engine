//
// Created by C66 on 2026/3/29.
//
// Semaphore RAII 包装类
//
#pragma once

#include <vulkan/vulkan.h>
#include <functional>

namespace engine::rhi::vulkan
{
    // Semaphore 包装类
    class Semaphore
    {
        public:
            Semaphore() = default;
            Semaphore(
                VkDevice                         device,
                VkSemaphore                      handle,
                bool                             isTimeline,
                std::function<void(VkSemaphore)> deleter);
            ~Semaphore();

            // 禁止拷贝
            Semaphore(const Semaphore&)            = delete;
            Semaphore& operator=(const Semaphore&) = delete;

            // 允许移动
            Semaphore(Semaphore&& other) noexcept;
            Semaphore& operator=(Semaphore&& other) noexcept;

            // 时间线信号量特有接口
            uint64_t GetCounterValue() const;
            void     Signal(uint64_t value) const;
            void     Wait(uint64_t value, uint64_t timeoutNs = UINT64_MAX) const;

            // 访问器
            VkSemaphore GetHandle() const { return handle_; }
            bool        IsTimeline() const { return isTimeline_; }
            explicit    operator bool() const { return handle_ != VK_NULL_HANDLE; }

            VkSemaphore Release();

        private:
            void Destroy();

            VkDevice                         device_     = VK_NULL_HANDLE;
            VkSemaphore                      handle_     = VK_NULL_HANDLE;
            bool                             isTimeline_ = false;
            std::function<void(VkSemaphore)> deleter_;
    };
} // namespace engine::rhi::vulkan
