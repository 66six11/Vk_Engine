#include "../../include/managers/SamplerManager.h"
#include <cassert>

namespace engine::rhi {

SamplerManager::SamplerManager(VkDevice device)
    : m_device(device) {
    assert(device != VK_NULL_HANDLE);
}

SamplerManager::~SamplerManager() {
    for (auto it = begin(); it != end(); ++it) {
        Sampler& sampler = *it;
        if (sampler.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device, sampler.sampler, nullptr);
        }
    }
    clear();
}

SamplerHandle SamplerManager::createSampler(const SamplerDesc& desc) {
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = desc.magFilter;
    createInfo.minFilter = desc.minFilter;
    createInfo.mipmapMode = desc.mipmapMode;
    createInfo.addressModeU = desc.addressModeU;
    createInfo.addressModeV = desc.addressModeV;
    createInfo.addressModeW = desc.addressModeW;
    createInfo.mipLodBias = desc.mipLodBias;
    createInfo.anisotropyEnable = desc.anisotropyEnable;
    createInfo.maxAnisotropy = desc.maxAnisotropy;
    createInfo.compareEnable = desc.compareEnable;
    createInfo.compareOp = desc.compareOp;
    createInfo.minLod = desc.minLod;
    createInfo.maxLod = desc.maxLod;
    createInfo.borderColor = desc.borderColor;
    createInfo.unnormalizedCoordinates = desc.unnormalizedCoordinates;

    Sampler sampler = {};
    VkResult result = vkCreateSampler(m_device, &createInfo, nullptr, &sampler.sampler);

    if (result != VK_SUCCESS) {
        return SamplerHandle();
    }

    return create(std::move(sampler));
}

} // namespace engine::rhi