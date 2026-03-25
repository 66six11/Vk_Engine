//
// Created by C66 on 2026/3/25.
//
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan/DeviceManager.h"

#include <stdexcept>
#include <vector>

#include "vulkan/VulkanUtils.h"
//辅助函数


// 创建实例
void CreateInstance(VkInstance& instance_);
//创建物理设备
void CreatePhysicalDevice(VkPhysicalDevice& physicalDevice_);
//创建逻辑设备
void CreateLogicalDevice(VkDevice& logicalDevice_);
//创建VMA内存
void CreateVMA(VmaAllocator& vmaAllocator_);

//设备评分
int RateDevice(VkPhysicalDevice device_);


void DeviceManager::Initialize()
{
    std::vector<const char*> requiredExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef VK_USE_PLATFORM_WIN32_KHR
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #endif
    };

    std::vector<const char*> validationLayers;
    #ifdef _DEBUG
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    #endif
    //查询可用扩展
    uint32_t extensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> extensions(extensionCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));
    
    // 创建 VkApplicationInfo
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_4;
    appInfo.pNext              = nullptr;

    // 创建 VkInstanceCreateInfo
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames     = validationLayers.data();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // 创建 VkInstance
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance_));

    // 查询物理设备
    uint32_t deviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr));
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()));
    
}
