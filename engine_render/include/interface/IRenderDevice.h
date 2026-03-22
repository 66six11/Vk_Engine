#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class ICommandBuffer;
class ICommandAllocator;
class IFence;
class ISemaphore;
class IPipelineState;
class IDescriptorPool;
class IDescriptorSetLayout;
class IHeap;

/**
 * 设备创建信息
 */
struct DeviceCreateInfo {
    RenderAPI api = RenderAPI::Vulkan;
    AdapterType adapterType = AdapterType::Discrete;
    FeatureLevel featureLevel = FeatureLevel::Level_1_0;

    // 窗口相关
    void* windowHandle = nullptr;
    ResourceExtent windowExtent = {800, 600};

    // 队列创建
    struct QueueCreateInfo {
        QueueType type = QueueType::Graphics;
        u32 count = 1;
        u32 priority = 1;
    };
    std::vector<QueueCreateInfo> queueCreateInfos;

    // 功能启用
    std::vector<std::string> requiredExtensions;
    std::vector<std::string> optionalExtensions;

    // 调试功能
    bool enableValidation = true;
    bool enableDebugMarkers = true;
    bool enableApiDump = false;

    // 多GPU支持
    u32 adapterIndex = 0;

    // 内存配置
    u64 deviceMemoryPoolSize = 256 * 1024 * 1024;  // 256MB
    u64 uploadMemoryPoolSize = 64 * 1024 * 1024;   // 64MB
    u64 readbackMemoryPoolSize = 16 * 1024 * 1024; // 16MB
};

/**
 * 渲染设备抽象接口
 * 负责底层GPU资源管理和命令提交
 */
class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    // 设备管理
    virtual bool initialize(const DeviceCreateInfo& createInfo) = 0;
    virtual void shutdown() = 0;

    // 设备特性
    virtual const DeviceInfo& getDeviceInfo() const = 0;
    virtual const AdapterInfo& getAdapterInfo() const = 0;
    virtual bool isFeatureSupported(Feature feature) const = 0;

    // 队列管理
    virtual ICommandQueue* getCommandQueue(QueueType type, u32 index = 0) = 0;
    virtual u32 getQueueCount(QueueType type) const = 0;

    // 命令分配器
    virtual ICommandAllocator* createCommandAllocator(QueueType queueType) = 0;
    virtual void destroyCommandAllocator(ICommandAllocator* allocator) = 0;

    // 命令缓冲区
    virtual ICommandBuffer* createCommandBuffer(ICommandAllocator* allocator,
                                                 CommandBufferLevel level) = 0;
    virtual void destroyCommandBuffer(ICommandBuffer* commandBuffer) = 0;

    // 同步对象
    virtual IFence* createFence(u64 initialValue = 0) = 0;
    virtual void destroyFence(IFence* fence) = 0;

    virtual ISemaphore* createSemaphore() = 0;
    virtual void destroySemaphore(ISemaphore* semaphore) = 0;

    // 查询池
    virtual QueryPoolHandle createQueryPool(u32 queryCount, bool occlusion = false) = 0;
    virtual void destroyQueryPool(QueryPoolHandle handle) = 0;

    // 描述符池
    virtual IDescriptorPool* createDescriptorPool(const struct DescriptorPoolDesc& desc) = 0;
    virtual void destroyDescriptorPool(IDescriptorPool* pool) = 0;

    // 描述符集布局
    virtual IDescriptorSetLayout* createDescriptorSetLayout(
        const struct DescriptorSetLayoutDesc& desc) = 0;
    virtual void destroyDescriptorSetLayout(IDescriptorSetLayout* layout) = 0;

    // 内存堆
    virtual IHeap* createHeap(const struct HeapDesc& desc) = 0;
    virtual void destroyHeap(IHeap* heap) = 0;

    // 设备控制
    virtual void waitIdle() = 0;
    virtual void flush() = 0;

    // 帧管理
    virtual u32 getCurrentFrameIndex() const = 0;
    virtual u32 getFrameCount() const = 0;
};

/**
 * 命令队列接口
 */
class ICommandQueue {
public:
    virtual ~ICommandQueue() = default;

    // 提交命令缓冲区
    virtual void submit(ICommandBuffer* commandBuffer,
                       u32 waitSemaphoreCount = 0,
                       ISemaphore** waitSemaphores = nullptr,
                       u64* waitValues = nullptr,
                       u32 signalSemaphoreCount = 0,
                       ISemaphore** signalSemaphores = nullptr,
                       u64* signalValues = nullptr,
                       IFence* fence = nullptr) = 0;

    // 批量提交
    virtual void submit(u32 commandBufferCount,
                       ICommandBuffer** commandBuffers,
                       u32 waitSemaphoreCount = 0,
                       ISemaphore** waitSemaphores = nullptr,
                       u64* waitValues = nullptr,
                       u32 signalSemaphoreCount = 0,
                       ISemaphore** signalSemaphores = nullptr,
                       u64* signalValues = nullptr,
                       IFence* fence = nullptr) = 0;

    // 等待
    virtual void waitIdle() = 0;
    virtual void waitFence(IFence* fence, u64 value) = 0;

    // 查询
    virtual QueueType getType() const = 0;
    virtual u32 getFamilyIndex() const = 0;
    virtual u32 getIndex() const = 0;

    // 时间戳
    virtual u64 getTimestampFrequency() const = 0;
    virtual void calibrateTimestamps(u64* gpuTimestamp, u64* cpuTimestamp) = 0;
};

/**
 * 命令分配器接口
 */
class ICommandAllocator {
public:
    virtual ~ICommandAllocator() = default;

    // 重置
    virtual void reset() = 0;

    // 查询
    virtual QueueType getQueueType() const = 0;
    virtual CommandBufferLevel getLevel() const = 0;
};

/**
 * 栅栏接口
 */
class IFence {
public:
    virtual ~IFence() = default;

    // 等待
    virtual void wait(u64 value) = 0;
    virtual bool wait(u64 value, u64 timeout) = 0;

    // 信号
    virtual void signal(u64 value) = 0;

    // 查询
    virtual u64 getCompletedValue() const = 0;
    virtual u64 getCurrentValue() const = 0;
};

/**
 * 信号量接口
 */
class ISemaphore {
public:
    virtual ~ISemaphore() = default;

    // 二进制信号量
    virtual void signal() = 0;
    virtual void wait() = 0;
    virtual bool isSignaled() const = 0;

    // 时间线信号量
    virtual void signal(u64 value) = 0;
    virtual void wait(u64 value) = 0;
    virtual u64 getValue() const = 0;
};

/**
 * 内存堆接口
 */
class IHeap {
public:
    virtual ~IHeap() = default;

    // 查询
    virtual u64 getSize() const = 0;
    virtual MemoryType getMemoryType() const = 0;
    virtual bool isDeviceLocal() const = 0;
    virtual bool isHostVisible() const = 0;
};

/**
 * 描述符池描述
 */
struct DescriptorPoolDesc {
    u32 maxSets = 1024;
    u32 maxSamplers = 1024;
    u32 maxSampledImages = 4096;
    u32 maxStorageImages = 1024;
    u32 maxUniformBuffers = 4096;
    u32 maxStorageBuffers = 1024;
    u32 maxUniformBuffersDynamic = 256;
    u32 maxStorageBuffersDynamic = 256;
    u32 maxInputAttachments = 256;
    bool freeIndividualSets = false;
};

/**
 * 描述符集布局绑定
 */
struct DescriptorSetLayoutBinding {
    u32 binding = 0;
    DescriptorType type = DescriptorType::UniformBuffer;
    u32 count = 1;
    ShaderStage stageFlags = ShaderStage::All;
    SamplerHandle immutableSampler;
};

/**
 * 描述符集布局描述
 */
struct DescriptorSetLayoutDesc {
    std::vector<DescriptorSetLayoutBinding> bindings;
    bool pushDescriptor = false;
    bool updateAfterBind = false;
};

/**
 * 内存堆描述
 */
struct HeapDesc {
    u64 size = 0;
    MemoryType memoryType = MemoryType::Default;
    u32 alignment = 0;
    bool deviceAddress = false;
};

} // namespace render