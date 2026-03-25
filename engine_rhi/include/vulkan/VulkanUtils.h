#pragma once

#ifndef ENGINE_VULKANUTILS_H
#define ENGINE_VULKANUTILS_H
#include <stdexcept>
#include <string>
#include <vulkan/vulkan.h>

namespace engine::rhi
{
    // 检查 VkResult，失败时抛出异常
    inline void CheckVkResult(VkResult result, const char* operation)
    {
        if (result != VK_SUCCESS)
        {
            //终止进程
            throw std::runtime_error(
                std::string("Vulkan operation '") + operation +
                "' failed with error code: " + std::to_string(result)
            );
        }
    }
} // namespace engine::rhi

// 辅助宏：将 __LINE__ 转为字符串
#define VK_STRINGIFY(x) #x
#define VK_TOSTRING(x) VK_STRINGIFY(x)

// 便捷宏：自动附加文件名和行号
#define VK_CHECK(x) engine::rhi::CheckVkResult(x, #x " at " __FILE__ ":" VK_TOSTRING(__LINE__))

#endif //ENGINE_VULKANUTILS_H
