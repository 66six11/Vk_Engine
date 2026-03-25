#pragma once
#ifndef ENGINE_RHI_SAMPLER_MANAGER_H
#define ENGINE_RHI_SAMPLER_MANAGER_H

#include "../rhi/Sampler.h"
#include "../core/ResourceManager.h"
#include <vulkan/vulkan.h>

namespace engine::rhi {

// 基础 Sampler 管理器
class SamplerManager : public ResourceManager<Sampler, SamplerHandle> {
public:
    explicit SamplerManager(VkDevice device);
    ~SamplerManager() override;

    SamplerManager(const SamplerManager&) = delete;
    SamplerManager& operator=(const SamplerManager&) = delete;

    // 创建 Sampler
    [[nodiscard]] SamplerHandle createSampler(const SamplerDesc& desc);

    // 获取 Vulkan Sampler
    [[nodiscard]] VkSampler getVkSampler(SamplerHandle handle) const {
        if (const Sampler* sampler = get(handle)) {
            return sampler->sampler;
        }
        return VK_NULL_HANDLE;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_SAMPLER_MANAGER_H