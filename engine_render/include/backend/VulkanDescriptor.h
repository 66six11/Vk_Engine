#pragma once

#include "interface/IRenderDevice.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;

    /**
 * Vulkan描述符池实现
 */
    class VulkanDescriptorPool : public IDescriptorPool {
        public:
            VulkanDescriptorPool(VulkanDevice* device);
            ~VulkanDescriptorPool() override;

            bool create(const DescriptorPoolDesc& desc);
            void destroy();

            // 分配描述符集
            VkDescriptorSet allocate(VkDescriptorSetLayout layout);
            void            free(VkDescriptorSet set);
            void            reset();

            // IDescriptorPool接口（简化版）
            u32 getMaxSets() const { return maxSets; }
            u32 getAllocatedSets() const { return allocatedSets; }

        private:
            VulkanDevice*    device;
            VkDescriptorPool pool           = nullptr;
            u32              maxSets        = 0;
            u32              allocatedSets  = 0;
            bool             individualFree = false;
    };

    /**
 * Vulkan描述符集布局实现
 */
    class VulkanDescriptorSetLayout : public IDescriptorSetLayout {
        public:
            VulkanDescriptorSetLayout(VulkanDevice* device);
            ~VulkanDescriptorSetLayout() override;

            bool create(const DescriptorSetLayoutDesc& desc);
            void destroy();

            VkDescriptorSetLayout          getLayout() const { return layout; }
            const DescriptorSetLayoutDesc& getDesc() const { return desc; }

            // 获取绑定信息
            const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const {
                return bindings;
            }

        private:
            VulkanDevice*                             device;
            VkDescriptorSetLayout                     layout = nullptr;
            DescriptorSetLayoutDesc                   desc;
            std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    /**
 * Vulkan描述符集管理器
 * 管理描述符集的分配和更新
 */
    class VulkanDescriptorSetManager {
        public:
            explicit VulkanDescriptorSetManager(VulkanDevice* device);
            ~VulkanDescriptorSetManager();

            bool initialize();
            void shutdown();

            // 获取或创建描述符集
            DescriptorSetHandle allocate(DescriptorSetLayoutHandle layout);
            void                free(DescriptorSetHandle handle);

            // 更新描述符集
            void updateBuffer(DescriptorSetHandle handle, u32     binding, BufferHandle buffer,
                              u64                 offset = 0, u64 range = ~0ull);
            void updateTexture(DescriptorSetHandle handle, u32 binding, TextureHandle texture,
                               SamplerHandle       sampler = {});
            void updateStorageImage(DescriptorSetHandle handle, u32 binding, TextureHandle texture);
            void updateAccelerationStructure(DescriptorSetHandle        handle, u32 binding,
                                             VkAccelerationStructureKHR as);

            // 批量更新
            struct Write {
                u32            binding;
                DescriptorType type;
                union {
                    struct { BufferHandle buffer; u64 offset; u64 range; }   bufferInfo;
                    struct { TextureHandle texture; SamplerHandle sampler; } imageInfo;
                    VkAccelerationStructureKHR                               accelerationStructure;
                };
            };
            void update(DescriptorSetHandle handle, const std::vector<Write>& writes);

            // 获取Vulkan描述符集
            VkDescriptorSet getVkDescriptorSet(DescriptorSetHandle handle);

            // 帧结束处理
            void nextFrame();

        private:
            VulkanDevice* device;

            // 描述符集存储
            struct DescriptorSet {
                VkDescriptorSet           set = nullptr;
                DescriptorSetLayoutHandle layout;
                VulkanDescriptorPool*     pool           = nullptr;
                u32                       frameAllocated = 0;
            };
            std::unordered_map<DescriptorSetHandle, DescriptorSet> sets;
            HandleAllocator<DescriptorSetHandle>                   handleAllocator;

            // 每帧描述符池
            std::vector<std::vector<std::unique_ptr<VulkanDescriptorPool>>> framePools;
            u32                                                             currentFrame      = 0;
            u32                                                             maxFramesInFlight = 3;

            // 描述符集布局缓存
            std::unordered_map<size_t, DescriptorSetLayoutHandle> layoutCache;

            // 辅助函数
            VulkanDescriptorPool* getOrCreatePool(DescriptorSetLayoutHandle layout);
            size_t                computeLayoutHash(const DescriptorSetLayoutDesc& desc);
    };

    /**
 * 动态描述符分配器
 * 用于每帧动态分配描述符集
 */
    class VulkanDynamicDescriptorAllocator {
        public:
            explicit VulkanDynamicDescriptorAllocator(VulkanDevice* device);
            ~VulkanDynamicDescriptorAllocator();

            void initialize(const std::vector<DescriptorPoolDesc>& poolSizes);
            void shutdown();

            // 分配描述符集
            VkDescriptorSet allocate(VkDescriptorSetLayout layout);

            // 重置当前帧的池
            void reset();

            // 下一帧
            void nextFrame();

        private:
            VulkanDevice* device;

            struct PoolEntry {
                std::unique_ptr<VulkanDescriptorPool> pool;
                u32                                   frameIndex;
            };

            std::vector<PoolEntry> pools;
            std::vector<PoolEntry> availablePools;
            u32                    currentFrame      = 0;
            u32                    maxFramesInFlight = 3;

            std::vector<DescriptorPoolDesc> poolConfigs;
    };

}
