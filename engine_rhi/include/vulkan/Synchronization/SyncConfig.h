//
// Created by C66 on 2026/3/29.
//
// 同步配置结构定义
//
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace engine::rhi::vulkan
{
    // ==================== 池配置结构 ====================

    // Fence 池配置
    struct FencePoolConfig
    {
        uint32_t initialCount   = 4;     // 初始 Fence 数量
        uint32_t maxCount       = 16;    // 最大 Fence 数量
        bool     createSignaled = false; // 初始状态是否已信号
    };

    // Semaphore 池配置
    struct SemaphorePoolConfig
    {
        uint32_t initialCount = 4;     // 初始信号量数量
        uint32_t maxCount     = 16;    // 最大信号量数量
        bool     timeline     = false; // 是否使用时间线信号量 (Vulkan 1.2+)
    };

    // ==================== 提交同步信息结构 ====================

    // 提交同步信息封装（支持普通信号量和时间线信号量）
    struct SubmitSyncInfo
    {
        // 等待信号量（GPU-GPU 同步）
        std::vector<VkSemaphore>          waitSemaphores;
        std::vector<VkPipelineStageFlags> waitStages;
        std::vector<uint64_t>             waitValues;  // 时间线信号量等待值（0表示二进制信号量）

        // 信号信号量（GPU-GPU 同步）
        std::vector<VkSemaphore> signalSemaphores;
        std::vector<uint64_t>    signalValues;  // 时间线信号量信号值（0表示二进制信号量）

        // Fence（CPU-GPU 同步）
        VkFence signalFence = VK_NULL_HANDLE;

        // 便捷方法 - 普通信号量
        void AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stage)
        {
            waitSemaphores.push_back(semaphore);
            waitStages.push_back(stage);
            waitValues.push_back(0);  // 二进制信号量值为0
        }

        void AddSignalSemaphore(VkSemaphore semaphore)
        {
            signalSemaphores.push_back(semaphore);
            signalValues.push_back(0);  // 二进制信号量值为0
        }

        // 便捷方法 - 时间线信号量
        void AddWaitTimelineSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stage, uint64_t value)
        {
            waitSemaphores.push_back(semaphore);
            waitStages.push_back(stage);
            waitValues.push_back(value);
        }

        void AddSignalTimelineSemaphore(VkSemaphore semaphore, uint64_t value)
        {
            signalSemaphores.push_back(semaphore);
            signalValues.push_back(value);
        }

        void SetFence(VkFence fence)
        {
            signalFence = fence;
        }

        // 检查方法
        bool HasWaits() const { return !waitSemaphores.empty(); }
        bool HasSignals() const { return !signalSemaphores.empty(); }
        bool HasFence() const { return signalFence != VK_NULL_HANDLE; }

        // 检查是否包含时间线信号量
        bool HasTimelineWaits() const
        {
            for (uint64_t value : waitValues)
            {
                if (value > 0) return true;
            }
            return false;
        }

        bool HasTimelineSignals() const
        {
            for (uint64_t value : signalValues)
            {
                if (value > 0) return true;
            }
            return false;
        }

        bool HasTimelineSemaphores() const { return HasTimelineWaits() || HasTimelineSignals(); }

        // 获取 VkTimelineSemaphoreSubmitInfo（用于构建提交信息）
        // 注意：返回的结构体生命周期与 SubmitSyncInfo 绑定
        VkTimelineSemaphoreSubmitInfo GetTimelineSemaphoreSubmitInfo() const
        {
            VkTimelineSemaphoreSubmitInfo timelineInfo = {};
            timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            timelineInfo.waitSemaphoreValueCount = static_cast<uint32_t>(waitValues.size());
            timelineInfo.pWaitSemaphoreValues = waitValues.empty() ? nullptr : waitValues.data();
            timelineInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalValues.size());
            timelineInfo.pSignalSemaphoreValues = signalValues.empty() ? nullptr : signalValues.data();
            return timelineInfo;
        }
    };

    // 呈现同步信息
    struct PresentSyncInfo
    {
        std::vector<VkSemaphore> waitSemaphores;
        VkFence                  completionFence = VK_NULL_HANDLE; // 可选

        void AddWaitSemaphore(VkSemaphore semaphore)
        {
            waitSemaphores.push_back(semaphore);
        }
    };
} // namespace engine::rhi::vulkan
