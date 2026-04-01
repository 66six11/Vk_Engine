#pragma once
#ifndef ENGINE_VULKAN_RESOURCE_HANDLES_H
#define ENGINE_VULKAN_RESOURCE_HANDLES_H

#include "../../core/TypedHandle.h"

namespace engine::rhi::vulkan
{
    // Vulkan资源类型的Tag定义
    struct VulkanBufferTag
    {
    };

    struct VulkanTextureTag
    {
    };

    struct VulkanSamplerTag
    {
    };

    struct VulkanShaderTag
    {
    };

    struct VulkanPipelineLayoutTag
    {
    };

    struct VulkanGraphicsPipelineTag
    {
    };

    struct VulkanComputePipelineTag
    {
    };

    // Vulkan资源Handle别名
    using VulkanBufferHandle           = TypedHandle<VulkanBufferTag>;
    using VulkanTextureHandle          = TypedHandle<VulkanTextureTag>;
    using VulkanSamplerHandle          = TypedHandle<VulkanSamplerTag>;
    using VulkanShaderHandle           = TypedHandle<VulkanShaderTag>;
    using VulkanPipelineLayoutHandle   = TypedHandle<VulkanPipelineLayoutTag>;
    using VulkanGraphicsPipelineHandle = TypedHandle<VulkanGraphicsPipelineTag>;
    using VulkanComputePipelineHandle  = TypedHandle<VulkanComputePipelineTag>;

    // 注意：BufferView/TextureView不暴露独立Handle
    // 直接返回VkBufferView/VkImageView，生命周期由上层或父资源Manager管理
} // namespace engine::rhi::vulkan

// std::hash特化
namespace std
{
    template <> struct hash<engine::rhi::vulkan::VulkanBufferHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanBufferHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanTextureHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanTextureHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanSamplerHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanSamplerHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanShaderHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanShaderHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanPipelineLayoutHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanPipelineLayoutHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanGraphicsPipelineHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanGraphicsPipelineHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };

    template <> struct hash<engine::rhi::vulkan::VulkanComputePipelineHandle>
    {
        [[nodiscard]] size_t operator()(const engine::rhi::vulkan::VulkanComputePipelineHandle& h) const noexcept
        {
            return std::hash<uint64_t>{}(h.value());
        }
    };
} // namespace std

#endif // ENGINE_VULKAN_RESOURCE_HANDLES_H
