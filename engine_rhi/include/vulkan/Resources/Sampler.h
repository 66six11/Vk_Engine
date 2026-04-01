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

        // 预设工厂方法（inline实现）
        [[nodiscard]] static SamplerDesc LinearWrap() {
            SamplerDesc desc;
            desc.magFilter = VK_FILTER_LINEAR;
            desc.minFilter = VK_FILTER_LINEAR;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            desc.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            return desc;
        }
        
        [[nodiscard]] static SamplerDesc LinearClamp() {
            SamplerDesc desc;
            desc.magFilter = VK_FILTER_LINEAR;
            desc.minFilter = VK_FILTER_LINEAR;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            return desc;
        }
        
        [[nodiscard]] static SamplerDesc NearestWrap() {
            SamplerDesc desc;
            desc.magFilter = VK_FILTER_NEAREST;
            desc.minFilter = VK_FILTER_NEAREST;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            desc.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            return desc;
        }
        
        [[nodiscard]] static SamplerDesc NearestClamp() {
            SamplerDesc desc;
            desc.magFilter = VK_FILTER_NEAREST;
            desc.minFilter = VK_FILTER_NEAREST;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            return desc;
        }
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
