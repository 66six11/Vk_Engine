#pragma once

#include "core/Types.h"

// Vulkan forward declarations
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkDevice_T;
struct VkQueue_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;
struct VkBuffer_T;
struct VkBufferView_T;
struct VkImage_T;
struct VkImageView_T;
struct VkSampler_T;
struct VkPipeline_T;
struct VkPipelineLayout_T;
struct VkDescriptorSet_T;
struct VkDescriptorSetLayout_T;
struct VkDescriptorPool_T;
struct VkFence_T;
struct VkSemaphore_T;
struct VkQueryPool_T;
struct VkRenderPass_T;
struct VkFramebuffer_T;
struct VkDeviceMemory_T;
struct VkSwapchainKHR_T;
struct VkSurfaceKHR_T;
struct VkDebugUtilsMessengerEXT_T;
struct VkAccelerationStructureKHR_T;

// Vulkan handles
using VkInstance = VkInstance_T*;
using VkPhysicalDevice = VkPhysicalDevice_T*;
using VkDevice = VkDevice_T*;
using VkQueue = VkQueue_T*;
using VkCommandPool = VkCommandPool_T*;
using VkCommandBuffer = VkCommandBuffer_T*;
using VkBuffer = VkBuffer_T*;
using VkBufferView = VkBufferView_T*;
using VkImage = VkImage_T*;
using VkImageView = VkImageView_T*;
using VkSampler = VkSampler_T*;
using VkPipeline = VkPipeline_T*;
using VkPipelineLayout = VkPipelineLayout_T*;
using VkDescriptorSet = VkDescriptorSet_T*;
using VkDescriptorSetLayout = VkDescriptorSetLayout_T*;
using VkDescriptorPool = VkDescriptorPool_T*;
using VkFence = VkFence_T*;
using VkSemaphore = VkSemaphore_T*;
using VkQueryPool = VkQueryPool_T*;
using VkRenderPass = VkRenderPass_T*;
using VkFramebuffer = VkFramebuffer_T*;
using VkDeviceMemory = VkDeviceMemory_T*;
using VkSwapchainKHR = VkSwapchainKHR_T*;
using VkSurfaceKHR = VkSurfaceKHR_T*;
using VkDebugUtilsMessengerEXT = VkDebugUtilsMessengerEXT_T*;
using VkAccelerationStructureKHR = VkAccelerationStructureKHR_T*;

namespace render::vulkan
{

    // Vulkan版本
    static constexpr u32 VulkanApiVersion = (1 << 22) | (3 << 12); // Vulkan 1.3

    // 扩展功能标志
    struct VulkanFeatures {
        bool swapchain               : 1;
        bool debugUtils              : 1;
        bool timelineSemaphore       : 1;
        bool descriptorIndexing      : 1;
        bool bufferDeviceAddress     : 1;
        bool memoryBudget            : 1;
        bool synchronization2        : 1;
        bool dynamicRendering        : 1;
        bool meshShader              : 1;
        bool rayTracing              : 1;
        bool accelerationStructure   : 1;
        bool deferredHostOperations  : 1;
        bool pipelineLibrary         : 1;
        bool graphicsPipelineLibrary : 1;
        bool shaderObject            : 1;
        bool maintenance4            : 1;
        bool maintenance5            : 1;
    };

    // 队列家族索引
    struct QueueFamilyIndices {
        i32 graphics      = -1;
        i32 compute       = -1;
        i32 transfer      = -1;
        i32 sparseBinding = -1;
        i32 videoDecode   = -1;
        i32 videoEncode   = -1;

        bool isComplete() const {
            return graphics >= 0;
        }
    };

    // Vulkan内存类型
    struct VulkanMemoryType {
        u32                   index;
        u32                   heapIndex;
        VkMemoryPropertyFlags propertyFlags;
        VkMemoryHeapFlags     heapFlags;
        u64                   heapSize;
    };

    // Vulkan格式属性
    struct VulkanFormatProperties {
        VkFormatFeatureFlags linearTilingFeatures;
        VkFormatFeatureFlags optimalTilingFeatures;
        VkFormatFeatureFlags bufferFeatures;
    };

}
