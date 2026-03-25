#pragma once
#ifndef ENGINE_RHI_SAMPLER_H
#define ENGINE_RHI_SAMPLER_H

#include <vulkan/vulkan.h>
#include <cstdint>

namespace engine::rhi {

// Sampler 描述信息
struct SamplerDesc {
    VkFilter magFilter = VK_FILTER_LINEAR;          // 放大过滤器
    VkFilter minFilter = VK_FILTER_LINEAR;          // 缩小过滤器
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Mipmap 模式
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // U 寻址模式
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // V 寻址模式
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // W 寻址模式
    float mipLodBias = 0.0f;                        // Mipmap LOD 偏移
    VkBool32 anisotropyEnable = VK_FALSE;           // 是否启用各向异性过滤
    float maxAnisotropy = 1.0f;                     // 最大各向异性
    VkBool32 compareEnable = VK_FALSE;              // 是否启用比较
    VkCompareOp compareOp = VK_COMPARE_OP_NEVER;    // 比较操作
    float minLod = 0.0f;                            // 最小 LOD
    float maxLod = VK_LOD_CLAMP_NONE;               // 最大 LOD
    VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    VkBool32 unnormalizedCoordinates = VK_FALSE;    // 是否使用非标准化坐标
    const char* name = nullptr;                     // 调试名称
};

// Sampler 资源结构
struct Sampler {
    VkSampler sampler = VK_NULL_HANDLE;             // Vulkan Sampler 对象

    // 检查是否有效
    [[nodiscard]] bool isValid() const { return sampler != VK_NULL_HANDLE; }
};

} // namespace engine::rhi

#endif // ENGINE_RHI_SAMPLER_H
