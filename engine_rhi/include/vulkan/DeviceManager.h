//
// Created by C66 on 2026/3/23.
//

#ifndef ENGINE_DEVICEMANAGER_H
#define ENGINE_DEVICEMANAGER_H

#include <vma/vk_mem_alloc.h>

#include "vulkan/vulkan.h"

class DeviceManager
{
    private:
        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
        VkDevice         logicalDevice_  = VK_NULL_HANDLE;
        VkInstance       instance_       = VK_NULL_HANDLE;
        // 调试工具（可选）
        VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;

        // Vulkan Memory Allocator
        VmaAllocator vmaAllocator_ = VK_NULL_HANDLE;
        
    public:
        
        void Initialize();
        void Shutdown();

        

        DeviceManager();
        ~DeviceManager();

        VkDevice GetLogicalDevice() const { return logicalDevice_; }
        VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice_; }
        VkInstance GetInstance() const { return instance_; }
        VmaAllocator GetAllocator() const { return vmaAllocator_; }

};


#endif //ENGINE_DEVICEMANAGER_H
