#include "../../include/managers/ShaderManager.h"
#include <fstream>
#include <vector>
#include <cassert>

namespace engine::rhi {

ShaderManager::ShaderManager(VkDevice device)
    : m_device(device) {
    assert(device != VK_NULL_HANDLE);
}

ShaderManager::~ShaderManager() {
    for (auto it = begin(); it != end(); ++it) {
        Shader& shader = *it;
        if (shader.module != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shader.module, nullptr);
        }
    }
    clear();
}

ShaderHandle ShaderManager::createShader(const ShaderDesc& desc) {
    if (!desc.code || desc.codeSize == 0) {
        return ShaderHandle();
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = desc.codeSize;
    createInfo.pCode = desc.code;

    Shader shader = {};
    shader.stage = desc.stage;
    shader.entryPoint = desc.entryPoint;

    VkResult result = vkCreateShaderModule(m_device, &createInfo, nullptr, &shader.module);

    if (result != VK_SUCCESS) {
        return ShaderHandle();
    }

    return create(std::move(shader));
}

ShaderHandle ShaderManager::loadShader(const std::string& filepath, ShaderStage stage) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ShaderHandle();
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    ShaderDesc desc = {};
    desc.stage = stage;
    desc.code = buffer.data();
    desc.codeSize = fileSize;

    return createShader(desc);
}

} // namespace engine::rhi