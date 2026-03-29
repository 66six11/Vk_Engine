//
// Created by C66 on 2026/3/29.
//
// Fence RAII 包装类
//
#pragma once

#include <vulkan/vulkan.h>
#include <functional>
#include <chrono>

namespace engine::rhi::vulkan
{
    // Fence 包装类
    class Fence
    {
        public:
            Fence() = default;
            Fence(VkDevice device, VkFence handle, std::function<void(VkFence)> deleter);
            ~Fence();

            // 禁止拷贝
            Fence(const Fence&)            = delete;
            Fence& operator=(const Fence&) = delete;

            // 允许移动
            Fence(Fence&& other) noexcept;
            Fence& operator=(Fence&& other) noexcept;

            // 状态查询
            bool IsSignaled() const;
            bool Wait(uint64_t timeoutNs = UINT64_MAX) const;
            bool WaitFor(std::chrono::milliseconds timeout) const;
            void Reset() const;

            // 访问器
            VkFence  GetHandle() const { return handle_; }
            explicit operator bool() const { return handle_ != VK_NULL_HANDLE; }

            // 内部使用：释放所有权（不归还给池）
            VkFence Release();

        private:
            void Destroy();

            VkDevice                     device_ = VK_NULL_HANDLE;
            VkFence                      handle_ = VK_NULL_HANDLE;
            std::function<void(VkFence)> deleter_; // 归还回调
    };
} // namespace engine::rhi::vulkan
