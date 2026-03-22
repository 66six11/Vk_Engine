#pragma once

#include "interface/IRenderDevice.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanCommandBuffer;
    class VulkanCommandAllocator;
    class VulkanFence;
    class VulkanSemaphore;
    class VulkanDescriptorPool;
    class VulkanDescriptorSetLayout;
    class VulkanHeap;
    class VulkanSwapChain;

    /**
 * Vulkan渲染设备实现
 */
    class VulkanDevice : public IRenderDevice {
        public:
            VulkanDevice();
            ~VulkanDevice() override;

            // IRenderDevice接口实现
            bool initialize(const DeviceCreateInfo& createInfo) override;
            void shutdown() override;

            const DeviceInfo&  getDeviceInfo() const override;
            const AdapterInfo& getAdapterInfo() const override;
            bool               isFeatureSupported(Feature feature) const override;

            ICommandQueue* getCommandQueue(QueueType type, u32 index = 0) override;
            u32            getQueueCount(QueueType type) const override;

            ICommandAllocator* createCommandAllocator(QueueType queueType) override;
            void               destroyCommandAllocator(ICommandAllocator* allocator) override;

            ICommandBuffer* createCommandBuffer(ICommandAllocator* allocator,
                                                CommandBufferLevel level) override;
            void destroyCommandBuffer(ICommandBuffer* commandBuffer) override;

            IFence* createFence(u64 initialValue = 0) override;
            void    destroyFence(IFence* fence) override;

            ISemaphore* createSemaphore() override;
            void        destroySemaphore(ISemaphore* semaphore) override;

            QueryPoolHandle createQueryPool(u32 queryCount, bool occlusion = false) override;
            void            destroyQueryPool(QueryPoolHandle handle) override;

            IDescriptorPool* createDescriptorPool(const DescriptorPoolDesc& desc) override;
            void             destroyDescriptorPool(IDescriptorPool* pool) override;

            IDescriptorSetLayout* createDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) override;
            void                  destroyDescriptorSetLayout(IDescriptorSetLayout* layout) override;

            IHeap* createHeap(const HeapDesc& desc) override;
            void   destroyHeap(IHeap* heap) override;

            void waitIdle() override;
            void flush() override;

            u32 getCurrentFrameIndex() const override;
            u32 getFrameCount() const override;

            // Vulkan特定方法
            VkInstance                getInstance() const { return instance; }
            VkPhysicalDevice          getPhysicalDevice() const { return physicalDevice; }
            VkDevice                  getDevice() const { return device; }
            const QueueFamilyIndices& getQueueFamilyIndices() const { return queueFamilies; }
            const VulkanFeatures&     getFeatures() const { return features; }

            u32                  findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) const;
            VkFormat             convertFormat(Format format) const;
            Format               convertFormat(VkFormat format) const;
            VkImageLayout        convertImageLayout(ImageLayout layout) const;
            ImageLayout          convertImageLayout(VkImageLayout layout) const;
            VkPipelineStageFlags convertPipelineStage(PipelineStage stage) const;
            VkAccessFlags        convertAccessFlags(AccessFlags access) const;
            VkImageAspectFlags   getImageAspectFlags(Format format) const;
            VkShaderStageFlags   convertShaderStage(ShaderStage stage) const;
            VkDescriptorType     convertDescriptorType(DescriptorType type) const;

            // 调试工具
            void setDebugObjectName(u64 object, VkObjectType objectType, const char* name);
            void beginDebugLabel(VkCommandBuffer cmd, const char* name, const Color& color);
            void endDebugLabel(VkCommandBuffer cmd);
            void insertDebugLabel(VkCommandBuffer cmd, const char* name, const Color& color);

        private:
            // Vulkan实例
            VkInstance               instance       = nullptr;
            VkPhysicalDevice         physicalDevice = nullptr;
            VkDevice                 device         = nullptr;
            VkDebugUtilsMessengerEXT debugMessenger = nullptr;

            // 队列
            QueueFamilyIndices   queueFamilies;
            std::vector<VkQueue> graphicsQueues;
            std::vector<VkQueue> computeQueues;
            std::vector<VkQueue> transferQueues;

            // 特性
            VulkanFeatures features;
            DeviceInfo     deviceInfo;
            AdapterInfo    adapterInfo;

            // 内存属性
            VkPhysicalDeviceMemoryProperties memoryProperties;

            // 帧管理
            u32 currentFrameIndex = 0;
            u32 frameCount        = 0;

            // 初始化辅助函数
            bool createInstance(const DeviceCreateInfo& createInfo);
            bool selectPhysicalDevice(const DeviceCreateInfo& createInfo);
            bool createLogicalDevice(const DeviceCreateInfo& createInfo);
            bool setupDebugMessenger();
            void queryDeviceInfo();
            void queryFeatures();

            // 队列家族查询
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

            // 扩展检查
            bool                     checkDeviceExtensionSupport(VkPhysicalDevice device);
            std::vector<const char*> getRequiredExtensions();
    };

    /**
 * Vulkan命令队列实现
 */
    class VulkanCommandQueue : public ICommandQueue {
        public:
            VulkanCommandQueue(VulkanDevice* device, VkQueue  queue, QueueType type,
                               u32           familyIndex, u32 index);
            ~VulkanCommandQueue() override = default;

            void submit(ICommandBuffer* commandBuffer,
                        u32             waitSemaphoreCount   = 0,
                        ISemaphore**    waitSemaphores       = nullptr,
                        u64*            waitValues           = nullptr,
                        u32             signalSemaphoreCount = 0,
                        ISemaphore**    signalSemaphores     = nullptr,
                        u64*            signalValues         = nullptr,
                        IFence*         fence                = nullptr) override;

            void submit(u32              commandBufferCount,
                        ICommandBuffer** commandBuffers,
                        u32              waitSemaphoreCount   = 0,
                        ISemaphore**     waitSemaphores       = nullptr,
                        u64*             waitValues           = nullptr,
                        u32              signalSemaphoreCount = 0,
                        ISemaphore**     signalSemaphores     = nullptr,
                        u64*             signalValues         = nullptr,
                        IFence*          fence                = nullptr) override;

            void waitIdle() override;
            void waitFence(IFence* fence, u64 value) override;

            QueueType getType() const override;
            u32       getFamilyIndex() const override;
            u32       getIndex() const override;

            u64  getTimestampFrequency() const override;
            void calibrateTimestamps(u64* gpuTimestamp, u64* cpuTimestamp) override;

            VkQueue getVkQueue() const { return queue; }

        private:
            VulkanDevice* device;
            VkQueue       queue;
            QueueType     type;
            u32           familyIndex;
            u32           queueIndex;
    };

    /**
 * Vulkan命令分配器实现
 */
    class VulkanCommandAllocator : public ICommandAllocator {
        public:
            VulkanCommandAllocator(VulkanDevice* device, VkCommandPool pool, QueueType type);
            ~VulkanCommandAllocator() override;

            void reset() override;

            QueueType          getQueueType() const override;
            CommandBufferLevel getLevel() const override;

            VkCommandPool getVkPool() const { return pool; }
            VulkanDevice* getDevice() const { return device; }

        private:
            VulkanDevice* device;
            VkCommandPool pool;
            QueueType     queueType;
    };

    /**
 * Vulkan栅栏实现
 */
    class VulkanFence : public IFence {
        public:
            VulkanFence(VulkanDevice* device, VkFence fence);
            ~VulkanFence() override;

            void wait(u64 value) override;
            bool wait(u64 value, u64 timeout) override;
            void signal(u64 value) override;
            u64  getCompletedValue() const override;
            u64  getCurrentValue() const override;

            VkFence getVkFence() const { return fence; }

        private:
            VulkanDevice* device;
            VkFence       fence;
            u64           currentValue = 0;
    };

    /**
 * Vulkan信号量实现
 */
    class VulkanSemaphore : public ISemaphore {
        public:
            VulkanSemaphore(VulkanDevice* device, VkSemaphore semaphore, bool timeline);
            ~VulkanSemaphore() override;

            void signal() override;
            void wait() override;
            bool isSignaled() const override;

            void signal(u64 value) override;
            void wait(u64 value) override;
            u64  getValue() const override;

            VkSemaphore getVkSemaphore() const { return semaphore; }
            bool        isTimeline() const { return timeline; }

        private:
            VulkanDevice* device;
            VkSemaphore   semaphore;
            bool          timeline;
            u64           value = 0;
    };

    /**
 * Vulkan内存堆实现
 */
    class VulkanHeap : public IHeap {
        public:
            VulkanHeap(VulkanDevice* device, VkDeviceMemory memory, u64 size, MemoryType type);
            ~VulkanHeap() override;

            u64        getSize() const override;
            MemoryType getMemoryType() const override;
            bool       isDeviceLocal() const override;
            bool       isHostVisible() const override;

            VkDeviceMemory getVkMemory() const { return memory; }
            void*          map();
            void           unmap();

        private:
            VulkanDevice*  device;
            VkDeviceMemory memory;
            u64            size;
            MemoryType     memoryType;
            void*          mappedPtr = nullptr;
    };

}
