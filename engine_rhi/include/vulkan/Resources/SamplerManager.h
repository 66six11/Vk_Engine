#pragma once
#ifndef ENGINE_VULKAN_SAMPLER_MANAGER_H
#define ENGINE_VULKAN_SAMPLER_MANAGER_H

#include "Sampler.h"
#include "VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <unordered_map>

namespace engine::rhi::vulkan
{
    // Sampler管理器 - 带缓存去重
    class SamplerManager : public ResourceManager<Sampler, VulkanSamplerHandle>
    {
        public:
            SamplerManager(VkDevice device);
            ~SamplerManager() override;

            SamplerManager(const SamplerManager&)            = delete;
            SamplerManager& operator=(const SamplerManager&) = delete;

            SamplerManager(SamplerManager&&) noexcept            = default;
            SamplerManager& operator=(SamplerManager&&) noexcept = default;

            // 创建Sampler（自动去重）
            [[nodiscard]] VulkanSamplerHandle createSampler(const SamplerDesc& desc);

            // 获取或创建（带缓存）
            [[nodiscard]] VulkanSamplerHandle getOrCreateSampler(const SamplerDesc& desc);

            // 获取Vulkan原始句柄
            [[nodiscard]] VkSampler getVkSampler(VulkanSamplerHandle handle) const;

            // 预定义常用Sampler（延迟创建）
            [[nodiscard]] VulkanSamplerHandle getLinearWrap();
            [[nodiscard]] VulkanSamplerHandle getLinearClamp();
            [[nodiscard]] VulkanSamplerHandle getNearestWrap();
            [[nodiscard]] VulkanSamplerHandle getNearestClamp();
            [[nodiscard]] VulkanSamplerHandle getAnisotropic(float maxAniso = 16.0f);

            // 清除缓存（可选，用于内存优化）
            void clearCache();

        private:
            // 计算SamplerDesc的哈希值
            static uint64_t computeHash(const SamplerDesc& desc);

            VkDevice                                          m_device;
            std::unordered_map<uint64_t, VulkanSamplerHandle> m_samplerCache;

            // 预定义Sampler缓存
            VulkanSamplerHandle m_linearWrap;
            VulkanSamplerHandle m_linearClamp;
            VulkanSamplerHandle m_nearestWrap;
            VulkanSamplerHandle m_nearestClamp;
            VulkanSamplerHandle m_anisotropic16;
    };
} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_SAMPLER_MANAGER_H
