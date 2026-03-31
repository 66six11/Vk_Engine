//
// Created by C66 on 2026/3/23.
//

#ifndef ENGINE_DEVICEMANAGER_H
#define ENGINE_DEVICEMANAGER_H

#include <vma/vk_mem_alloc.h>
#include <vector>

#include "vulkan/vulkan.h"
#include "vulkan/DeviceCapabilities.h"
#include "vulkan/Command/CommandQueue.h"

namespace engine::rhi::vulkan
{
    // 队列族索引结构体
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily = UINT32_MAX; // 图形队列族
        uint32_t computeFamily  = UINT32_MAX; // 计算队列族
        uint32_t transferFamily = UINT32_MAX; // 传输队列族

        // 检查是否所有必需队列都可用
        bool IsComplete() const
        {
            return graphicsFamily != UINT32_MAX &&
                   computeFamily != UINT32_MAX &&
                   transferFamily != UINT32_MAX;
        }

        // 检查队列是否共享同一队列族
        bool IsUnifiedQueue() const
        {
            return graphicsFamily == computeFamily && computeFamily == transferFamily;
        }

        // 检查是否所有队列都是独立的
        bool HasDedicatedQueues() const
        {
            return graphicsFamily != computeFamily &&
                   graphicsFamily != transferFamily &&
                   computeFamily != transferFamily;
        }
    };

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

            // 队列族索引
            QueueFamilyIndices queueFamilyIndices_;

            // 队列句柄
            VkQueue graphicsQueue_ = VK_NULL_HANDLE;
            VkQueue computeQueue_  = VK_NULL_HANDLE;
            VkQueue transferQueue_ = VK_NULL_HANDLE;

        public:
            void Initialize();
            void Shutdown();

            DeviceManager();
            ~DeviceManager();
            //禁止拷贝
            DeviceManager(const DeviceManager&)            = delete;
            DeviceManager& operator=(const DeviceManager&) = delete;
            //禁止移动
            DeviceManager(DeviceManager&&)            = delete;
            DeviceManager& operator=(DeviceManager&&) = delete;


            VkDevice         GetLogicalDevice() const { return logicalDevice_; }
            VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice_; }
            VkInstance       GetInstance() const { return instance_; }
            VmaAllocator     GetAllocator() const { return vmaAllocator_; }

            // 队列族索引访问
            const QueueFamilyIndices& GetQueueFamilyIndices() const { return queueFamilyIndices_; }
            uint32_t                  GetGraphicsQueueFamily() const { return queueFamilyIndices_.graphicsFamily; }
            uint32_t                  GetComputeQueueFamily() const { return queueFamilyIndices_.computeFamily; }
            uint32_t                  GetTransferQueueFamily() const { return queueFamilyIndices_.transferFamily; }

            // 队列句柄访问
            VkQueue GetGraphicsQueue() const { return graphicsQueue_; }
            VkQueue GetComputeQueue() const { return computeQueue_; }
            VkQueue GetTransferQueue() const { return transferQueue_; }

            // ==================== 设备能力访问 ====================

            // 获取完整的设备能力信息
            const GpuDeviceCaps& GetCapabilities() const { return capabilities_; }

            // 便捷方法：检查特定功能支持

            // ==================== 队列访问 ====================

            // 创建图形队列包装器
            CommandQueue CreateGraphicsQueue() const { return CommandQueue(graphicsQueue_, queueFamilyIndices_.graphicsFamily, 0); }

            // 创建计算队列包装器
            CommandQueue CreateComputeQueue() const { return CommandQueue(computeQueue_, queueFamilyIndices_.computeFamily, 0); }

            // 创建传输队列包装器
            CommandQueue CreateTransferQueue() const { return CommandQueue(transferQueue_, queueFamilyIndices_.transferFamily, 0); }

            // 等待图形队列空闲（便捷方法）
            void WaitGraphicsQueueIdle() const;
            // 检查设备支持的功能
            bool SupportsTimelineSemaphore() const { return capabilities_.timelineSemaphore; }
            bool SupportsDynamicRendering() const { return capabilities_.dynamicRendering; }
            bool SupportsBufferDeviceAddress() const { return capabilities_.bufferDeviceAddress; }
            bool SupportsDescriptorIndexing() const { return capabilities_.descriptorIndexing; }
            bool SupportsSynchronization2() const { return capabilities_.synchronization2; }
            bool SupportsMeshShader() const { return capabilities_.meshShader; }
            bool SupportsRayTracing() const { return capabilities_.SupportsRayTracing(); }
            bool SupportsBindless() const { return capabilities_.SupportsBindless(); }

        private:
            // ==================== 内部方法 ====================

            void CreateInstance();
            void CreatePhysicalDevice();
            void CreateLogicalDevice();
            void CreateVMA();

            void               QueryGpuDeviceCaps();
            QueueFamilyIndices FindQueueFamilies();
            static int                RateDevice(VkPhysicalDevice device);

        private:
            GpuDeviceCaps capabilities_;
    };
} // namespace engine::rhi::vulkan

#endif //ENGINE_DEVICEMANAGER_H
