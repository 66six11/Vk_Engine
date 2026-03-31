//
// Created by C66 on 2026/3/29.
//
// 二进制 Semaphore RAII 包装类
//
#pragma once

#include <vulkan/vulkan.h>
#include <functional>

namespace engine::rhi::vulkan
{
    // 二进制 Semaphore 包装类
    class Semaphore
    {
        public:
            Semaphore() = default;
            Semaphore(
                VkDevice                         device,
                VkSemaphore                      handle,
                std::function<void(VkSemaphore)> deleter);
            ~Semaphore();

            // 禁止拷贝
            Semaphore(const Semaphore&)            = delete;
            Semaphore& operator=(const Semaphore&) = delete;

            // 允许移动
            Semaphore(Semaphore&& other) noexcept;
            Semaphore& operator=(Semaphore&& other) noexcept;

            // 访问器
            VkSemaphore GetHandle() const { return handle_; }
            explicit    operator bool() const { return handle_ != VK_NULL_HANDLE; }

            VkSemaphore Release();

        private:
            void Destroy();

            VkDevice                         device_ = VK_NULL_HANDLE;
            VkSemaphore                      handle_ = VK_NULL_HANDLE;
            std::function<void(VkSemaphore)> deleter_;
    };
} // namespace engine::rhi::vulkan
