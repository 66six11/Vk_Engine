#pragma once
#ifndef ENGINE_RHI_SHADER_MANAGER_H
#define ENGINE_RHI_SHADER_MANAGER_H

#include "../rhi/Shader.h"
#include "../core/ResourceManager.h"
#include <vulkan/vulkan.h>

namespace engine::rhi {

// 基础 Shader 管理器 - 只提供创建/销毁功能
class ShaderManager : public ResourceManager<Shader, ShaderHandle> {
public:
    explicit ShaderManager(VkDevice device);
    ~ShaderManager() override;

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    // 从内存创建 Shader
    [[nodiscard]] ShaderHandle createShader(const ShaderDesc& desc);

    // 从文件加载 Shader
    [[nodiscard]] ShaderHandle loadShader(const std::string& filepath, ShaderStage stage);

    // 获取 Vulkan ShaderModule
    [[nodiscard]] VkShaderModule getVkModule(ShaderHandle handle) const {
        if (const Shader* shader = get(handle)) {
            return shader->module;
        }
        return VK_NULL_HANDLE;
    }

private:
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_SHADER_MANAGER_H