//
// Created by C66 on 2026/3/25.
//
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif

// VMA 实现定义（必须在一个 .cpp 文件中定义）
#define VMA_IMPLEMENTATION

#include "vulkan/DeviceManager.h"

#include <stdexcept>
#include <vector>
#include <cstring>

#include "vulkan/VulkanUtils.h"

namespace engine::rhi::vulkan
{
    // ==================== 静态扩展列表 ====================

    // 所需实例扩展
    static std::vector<const char*> requiredExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef VK_USE_PLATFORM_WIN32_KHR
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        #endif
    };

    // 所需设备扩展
    static std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // 可选设备扩展（根据能力动态添加）
    static std::vector<const char*> optionalDeviceExtensions = {
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    };

    // 验证层
    static std::vector<const char*> validationLayers;

    // ==================== 构造函数/析构函数 ====================

    DeviceManager::DeviceManager()  = default;
    DeviceManager::~DeviceManager() = default;

    // ==================== 主流程 ====================

    void DeviceManager::Initialize()
    {
        CreateInstance();
        CreatePhysicalDevice();

        // 查询设备能力（在创建设备前）
        QueryDeviceCapabilities();

        CreateLogicalDevice();

        CreateVMA();
    }

    void DeviceManager::Shutdown()
    {
        // 依次销毁
        vmaDestroyAllocator(vmaAllocator_);
        vkDestroyDevice(logicalDevice_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    // ==================== 实例创建 ====================

    void DeviceManager::CreateInstance()
    {
        #ifdef _DEBUG
        validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        #endif
        // 查询可用扩展
        uint32_t extensionCount = 0;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));

        // 检查必需的扩展是否都支持
        for (const char* required : requiredExtensions)
        {
            bool found = false;
            for (const auto& available : extensions)
            {
                if (strcmp(required, available.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                throw std::runtime_error(std::string("Required extension not supported: ") + required);
            }
        }

        // 创建 VkApplicationInfo
        VkApplicationInfo appInfo  = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = "TechArt Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "TechArt";
        appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_3; // 请求 Vulkan 1.3
        appInfo.pNext              = nullptr;

        // 创建 VkInstanceCreateInfo
        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledLayerCount       = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames     = validationLayers.data();
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        // 创建 VkInstance
        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance_));
    }

    // ==================== 物理设备选择 ====================

    void DeviceManager::CreatePhysicalDevice()
    {
        // 查询物理设备
        uint32_t deviceCount = 0;
        VK_CHECK(vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr));
        if (deviceCount == 0)
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()));

        // 选择评分最高的设备
        physicalDevice_ = devices[0];
        for (uint32_t i = 0; i < deviceCount; i++)
        {
            if (RateDevice(devices[i]) > RateDevice(physicalDevice_))
            {
                physicalDevice_ = devices[i];
            }
        }
    }

    int DeviceManager::RateDevice(VkPhysicalDevice device_)
    {
        int score = 0;

        // 查询设备支持的扩展
        uint32_t extensionCount = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device_, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(device_, nullptr, &extensionCount, availableExtensions.data()));

        // 检查必需的设备扩展是否都支持
        for (const char* required : requiredDeviceExtensions)
        {
            bool found = false;
            for (const auto& available : availableExtensions)
            {
                if (strcmp(required, available.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }
            // 如果不支持必需的扩展，直接返回0分（排除该设备）
            if (!found)
            {
                return 0;
            }
        }

        // 获取设备属性
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device_, &properties);

        // 根据设备类型评分（独显优先）
        switch (properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 1000; // 独立显卡
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 500; // 集成显卡
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                score += 300; // 虚拟GPU
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                score += 100; // CPU软渲染
                break;
            default:
                score += 1; // 其他未知类型
                break;
        }

        // 根据显存大小加分
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device_, &memProperties);

        VkDeviceSize totalMemory = 0;
        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++)
        {
            if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                totalMemory += memProperties.memoryHeaps[i].size;
            }
        }
        // 每GB显存加10分
        score += static_cast<int>(totalMemory / (1024 * 1024 * 1024)) * 10;

        return score;
    }

    // ==================== 设备能力查询 ====================

    void DeviceManager::QueryDeviceCapabilities()
    {
        // 获取物理设备属性
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

        // 获取 Vulkan 1.2+ 特性
        VkPhysicalDeviceFeatures2 features2 = {};
        features2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // 链接各种特性结构
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures = {};
        timelineFeatures.sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        dynamicRenderingFeatures.sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
        bufferDeviceAddressFeatures.sType                                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;

        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
        descriptorIndexingFeatures.sType                                      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        VkPhysicalDeviceSynchronization2Features sync2Features = {};
        sync2Features.sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

        // 构建 pNext 链
        features2.pNext                   = &timelineFeatures;
        timelineFeatures.pNext            = &dynamicRenderingFeatures;
        dynamicRenderingFeatures.pNext    = &bufferDeviceAddressFeatures;
        bufferDeviceAddressFeatures.pNext = &descriptorIndexingFeatures;
        descriptorIndexingFeatures.pNext  = &sync2Features;

        vkGetPhysicalDeviceFeatures2(physicalDevice_, &features2);

        // 填充能力结构
        capabilities_.timelineSemaphore   = (timelineFeatures.timelineSemaphore == VK_TRUE);
        capabilities_.dynamicRendering    = (dynamicRenderingFeatures.dynamicRendering == VK_TRUE);
        capabilities_.bufferDeviceAddress = (bufferDeviceAddressFeatures.bufferDeviceAddress == VK_TRUE);
        capabilities_.descriptorIndexing  = (descriptorIndexingFeatures.descriptorBindingPartiallyBound == VK_TRUE);
        capabilities_.synchronization2    = (sync2Features.synchronization2 == VK_TRUE);

        // 检查扩展支持（网格着色器、光追等）
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, extensions.data());

        auto hasExtension = [&](const char* name) -> bool
        {
            for (const auto& ext : extensions)
            {
                if (strcmp(ext.extensionName, name) == 0)
                    return true;
            }
            return false;
        };

        // 可选扩展检查
        capabilities_.meshShader            = hasExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        capabilities_.rayTracingPipeline    = hasExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        capabilities_.accelerationStructure = hasExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        capabilities_.rayQuery              = hasExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    }

    // ==================== 逻辑设备创建 ====================

    QueueFamilyIndices DeviceManager::FindQueueFamilies()
    {
        QueueFamilyIndices indices;

        // 查询队列族属性
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

        // 第一步：查找专用的传输队列族（只支持传输，性能最好）
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilies[i];
            // 专用传输队列：只支持传输，不支持图形和计算
            if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.transferFamily = i;
                break;
            }
        }

        // 第二步：查找专用的计算队列族（只支持计算）
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilies[i];
            // 专用计算队列：支持计算但不支持图形
            if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.computeFamily = i;
                break;
            }
        }

        // 第三步：查找图形队列族
        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilies[i];
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                break;
            }
        }

        // 回退策略：如果没有找到专用队列，使用图形队列族
        if (indices.computeFamily == UINT32_MAX && indices.graphicsFamily != UINT32_MAX)
        {
            indices.computeFamily = indices.graphicsFamily;
        }
        if (indices.transferFamily == UINT32_MAX && indices.graphicsFamily != UINT32_MAX)
        {
            indices.transferFamily = indices.graphicsFamily;
        }

        return indices;
    }

    void DeviceManager::CreateLogicalDevice()
    {
        // 查找队列族
        queueFamilyIndices_ = FindQueueFamilies();

        if (!queueFamilyIndices_.IsComplete())
        {
            throw std::runtime_error("Failed to find complete queue families!");
        }

        // 收集唯一的队列族（避免重复创建）
        std::vector<uint32_t> uniqueQueueFamilies;
        uniqueQueueFamilies.push_back(queueFamilyIndices_.graphicsFamily);
        if (queueFamilyIndices_.computeFamily != queueFamilyIndices_.graphicsFamily)
        {
            uniqueQueueFamilies.push_back(queueFamilyIndices_.computeFamily);
        }
        if (queueFamilyIndices_.transferFamily != queueFamilyIndices_.graphicsFamily &&
            queueFamilyIndices_.transferFamily != queueFamilyIndices_.computeFamily)
        {
            uniqueQueueFamilies.push_back(queueFamilyIndices_.transferFamily);
        }

        // 创建队列创建信息
        float                                queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex        = queueFamily;
            queueCreateInfo.queueCount              = 1;
            queueCreateInfo.pQueuePriorities        = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // ==================== 构建特性链 ====================

        // 基础特性
        VkPhysicalDeviceFeatures2 features2 = {};
        features2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

        // 各特性结构
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures = {};
        timelineFeatures.sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        dynamicRenderingFeatures.sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
        bufferDeviceAddressFeatures.sType                                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;

        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
        descriptorIndexingFeatures.sType                                      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        VkPhysicalDeviceSynchronization2Features sync2Features = {};
        sync2Features.sType                                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;

        // 构建 pNext 链
        void** ppNext = &features2.pNext;

        // 启用时间线信号量
        if (capabilities_.timelineSemaphore)
        {
            timelineFeatures.timelineSemaphore = VK_TRUE;
            *ppNext                            = &timelineFeatures;
            ppNext                             = &timelineFeatures.pNext;
        }

        // 启用动态渲染
        if (capabilities_.dynamicRendering)
        {
            dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
            *ppNext                                   = &dynamicRenderingFeatures;
            ppNext                                    = &dynamicRenderingFeatures.pNext;
        }

        // 启用缓冲设备地址
        if (capabilities_.bufferDeviceAddress)
        {
            bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
            *ppNext                                         = &bufferDeviceAddressFeatures;
            ppNext                                          = &bufferDeviceAddressFeatures.pNext;
        }

        // 启用描述符索引
        if (capabilities_.descriptorIndexing)
        {
            descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
            descriptorIndexingFeatures.runtimeDescriptorArray          = VK_TRUE;
            *ppNext                                                    = &descriptorIndexingFeatures;
            ppNext                                                     = &descriptorIndexingFeatures.pNext;
        }

        // 启用同步2
        if (capabilities_.synchronization2)
        {
            sync2Features.synchronization2 = VK_TRUE;
            *ppNext                        = &sync2Features;
        }

        // 消除未使用变量的警告（ppNext 在链式构建中使用）
        (void)ppNext;

        // ==================== 构建扩展列表 ====================

        std::vector<const char*> enabledExtensions = requiredDeviceExtensions;

        // 根据能力启用可选扩展
        if (capabilities_.dynamicRendering)
            enabledExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        if (capabilities_.timelineSemaphore)
            enabledExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        if (capabilities_.bufferDeviceAddress)
            enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        if (capabilities_.descriptorIndexing)
            enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        if (capabilities_.synchronization2)
            enabledExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

        // ==================== 创建设备 ====================

        VkDeviceCreateInfo createInfo      = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext                   = &features2; // 链接特性链
        createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos       = queueCreateInfos.data();
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();

        // 在调试模式下启用验证层
        #ifdef _DEBUG
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        #endif

        // 创建逻辑设备
        VK_CHECK(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_));

        // 获取各队列句柄
        vkGetDeviceQueue(logicalDevice_, queueFamilyIndices_.graphicsFamily, 0, &graphicsQueue_);
        vkGetDeviceQueue(logicalDevice_, queueFamilyIndices_.computeFamily, 0, &computeQueue_);
        vkGetDeviceQueue(logicalDevice_, queueFamilyIndices_.transferFamily, 0, &transferQueue_);
    }

    // ==================== VMA 创建 ====================

    void DeviceManager::CreateVMA()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice         = physicalDevice_;
        allocatorInfo.device                 = logicalDevice_;
        allocatorInfo.instance               = instance_;

        // 如果支持缓冲设备地址，启用 VMA 对应标志
        if (capabilities_.bufferDeviceAddress)
        {
            allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }

        VK_CHECK(vmaCreateAllocator(&allocatorInfo, &vmaAllocator_));
    }
} // namespace engine::rhi::vulkan
