#pragma once
#ifndef ENGINE_VULKAN_SHADER_MANAGER_H
#define ENGINE_VULKAN_SHADER_MANAGER_H

#include "Shader.h"
#include "VulkanResourceHandles.h"
#include "../../core/ResourceManager.h"
#include <string>

namespace engine::rhi::vulkan {

class ShaderManager : public ResourceManager<Shader, VulkanShaderHandle> {
public:
    explicit ShaderManager(VkDevice device);
    ~ShaderManager() override;

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    ShaderManager(ShaderManager&&) noexcept = default;
    ShaderManager& operator=(ShaderManager&&) noexcept = default;

    // 从内存创建Shader（SPIR-V字节码）
    [[nodiscard]] VulkanShaderHandle createShader(const ShaderDesc& desc);

    // 从文件加载Shader
    [[nodiscard]] VulkanShaderHandle loadShader(const std::string& filepath,
                                                 ShaderStage stage,
                                                 const std::string& entryPoint = "main");

    // 获取Vulkan原始句柄
    [[nodiscard]] VkShaderModule getVkShaderModule(VulkanShaderHandle handle) const;
    [[nodiscard]] ShaderStage getShaderStage(VulkanShaderHandle handle) const;
    [[nodiscard]] const std::string& getEntryPoint(VulkanShaderHandle handle) const;

    // 获取PipelineShaderStageCreateInfo
    [[nodiscard]] VkPipelineShaderStageCreateInfo getPipelineStageInfo(VulkanShaderHandle handle) const;

private:
    VkDevice m_device;
};

} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_SHADER_MANAGER_H
