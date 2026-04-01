#pragma once
#ifndef ENGINE_VULKAN_SAMPLER_H
#define ENGINE_VULKAN_SAMPLER_H

#include "VulkanResources.h"
#include <string>

namespace engine::rhi::vulkan
{
    struct SamplerDesc
    {
        VkFilter             magFilter               = VK_FILTER_LINEAR;
        VkFilter             minFilter               = VK_FILTER_LINEAR;
        VkSamplerMipmapMode  mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        float                mipLodBias              = 0.0f;
        VkBool32             anisotropyEnable        = VK_FALSE;
        float                maxAnisotropy           = 16.0f;
        VkBool32             compareEnable           = VK_FALSE;
        VkCompareOp          compareOp               = VK_COMPARE_OP_ALWAYS;
        float                minLod                  = 0.0f;
        float                maxLod                  = VK_LOD_CLAMP_NONE;
        VkBorderColor        borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        VkBool32             unnormalizedCoordinates = VK_FALSE; // 通常false
        const char*          debugName               = nullptr;

        // 预设工厂方法（实现在cpp中）
        [[nodiscard]] static SamplerDesc LinearWrap();
        [[nodiscard]] static SamplerDesc LinearClamp();
        [[nodiscard]] static SamplerDesc NearestWrap();
        [[nodiscard]] static SamplerDesc NearestClamp();
        [[nodiscard]] static SamplerDesc Anisotropic(float maxAniso = 16.0f);
        [[nodiscard]] static SamplerDesc ShadowPCF(); // 阴影比较采样
    };

    struct Sampler
    {
        VkSampler   sampler = VK_NULL_HANDLE;
        SamplerDesc desc;
        uint64_t    hash = 0; // 用于去重缓存

        [[nodiscard]] bool isValid() const { return sampler != VK_NULL_HANDLE; }
    };
} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_SAMPLER_H
