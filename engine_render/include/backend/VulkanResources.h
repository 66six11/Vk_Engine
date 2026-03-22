#pragma once

#include "interface/IResourceManager.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;
    class VulkanHeap;

    /**
 * Vulkan纹理实现
 */
    class VulkanTexture {
        public:
            VulkanTexture(VulkanDevice* device);
            ~VulkanTexture();

            bool create(const TextureCreateInfo& createInfo);
            void destroy();

            // 获取Vulkan对象
            VkImage        getImage() const { return image; }
            VkImageView    getImageView() const { return imageView; }
            VkDeviceMemory getMemory() const { return memory; }

            // 获取信息
            const TextureInfo& getInfo() const { return info; }
            VkImageLayout      getCurrentLayout() const { return currentLayout; }
            void               setCurrentLayout(VkImageLayout layout) { currentLayout = layout; }

            // 子资源布局
            VkImageLayout getSubresourceLayout(u32 mipLevel, u32 arrayLayer) const;
            void          setSubresourceLayout(u32 mipLevel, u32 arrayLayer, VkImageLayout layout);

            // 内存管理
            bool allocateMemory(const VkMemoryRequirements& requirements, MemoryType memoryType);
            void bindMemory(VulkanHeap* heap, u64 offset);

        private:
            VulkanDevice*  device;
            VkImage        image      = nullptr;
            VkImageView    imageView  = nullptr;
            VkDeviceMemory memory     = nullptr;
            VulkanHeap*    heap       = nullptr;
            u64            heapOffset = 0;

            TextureInfo                info;
            VkImageLayout              currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            std::vector<VkImageLayout> subresourceLayouts;
    };

    /**
 * Vulkan缓冲区实现
 */
    class VulkanBuffer {
        public:
            VulkanBuffer(VulkanDevice* device);
            ~VulkanBuffer();

            bool create(const BufferCreateInfo& createInfo);
            void destroy();

            // 获取Vulkan对象
            VkBuffer       getBuffer() const { return buffer; }
            VkBufferView   getBufferView() const { return bufferView; }
            VkDeviceMemory getMemory() const { return memory; }

            // 获取信息
            const BufferInfo& getInfo() const { return info; }

            // 内存管理
            bool allocateMemory(const VkMemoryRequirements& requirements, MemoryType memoryType);
            void bindMemory(VulkanHeap* heap, u64 offset);

            // 映射
            void* map();
            void  unmap();
            bool  isMapped() const { return mappedPtr != nullptr; }

            // 设备地址
            u64 getDeviceAddress() const;

        private:
            VulkanDevice*  device;
            VkBuffer       buffer     = nullptr;
            VkBufferView   bufferView = nullptr;
            VkDeviceMemory memory     = nullptr;
            VulkanHeap*    heap       = nullptr;
            u64            heapOffset = 0;

            BufferInfo info;
            void*      mappedPtr     = nullptr;
            u64        deviceAddress = 0;
    };

    /**
 * Vulkan采样器实现
 */
    class VulkanSampler {
        public:
            VulkanSampler(VulkanDevice* device);
            ~VulkanSampler();

            bool create(const SamplerDesc& desc);
            void destroy();

            VkSampler getSampler() const { return sampler; }

        private:
            VulkanDevice* device;
            VkSampler     sampler = nullptr;
    };

    /**
 * Vulkan资源视图实现
 */
    class VulkanResourceView {
        public:
            VulkanResourceView(VulkanDevice* device);
            ~VulkanResourceView();

            bool createForTexture(TextureHandle texture, const TextureViewDesc& desc);
            bool createForBuffer(BufferHandle buffer, const BufferViewDesc& desc);
            void destroy();

            VkImageView            getImageView() const { return imageView; }
            VkBufferView           getBufferView() const { return bufferView; }
            VkDescriptorImageInfo  getDescriptorImageInfo() const;
            VkDescriptorBufferInfo getDescriptorBufferInfo() const;

            ResourceViewType getType() const { return type; }
            Format           getFormat() const { return format; }

        private:
            VulkanDevice* device;
            VkImageView   imageView  = nullptr;
            VkBufferView  bufferView = nullptr;

            ResourceViewType type;
            Format           format;
            TextureHandle    texture;
            BufferHandle     buffer;

            VkDescriptorImageInfo  imageInfo;
            VkDescriptorBufferInfo bufferInfo;
    };

    /**
 * Vulkan资源管理器实现
 */
    class VulkanResourceManager : public IResourceManager {
        public:
            explicit VulkanResourceManager(VulkanDevice* device);
            ~VulkanResourceManager() override;

            bool initialize();
            void shutdown();

            // IResourceManager接口实现
            TextureHandle createTexture(const TextureCreateInfo& createInfo) override;
            TextureHandle createTextureFromData(const void*              data,
                                                const TextureCreateInfo& createInfo,
                                                u32                      rowPitch = 0) override;
            void        destroyTexture(TextureHandle handle) override;
            TextureInfo getTextureInfo(TextureHandle handle) const override;

            BufferHandle createBuffer(const BufferCreateInfo& createInfo) override;
            BufferHandle createBufferFromData(const void*             data, u64 size,
                                              const BufferCreateInfo& createInfo) override;
            void       destroyBuffer(BufferHandle handle) override;
            BufferInfo getBufferInfo(BufferHandle handle) const override;

            SamplerHandle createSampler(const SamplerDesc& desc) override;
            void          destroySampler(SamplerHandle handle) override;

            ResourceViewHandle createTextureView(TextureHandle          texture,
                                                 const TextureViewDesc& desc = {}) override;
            ResourceViewHandle createBufferView(BufferHandle          buffer,
                                                const BufferViewDesc& desc = {}) override;
            void destroyResourceView(ResourceViewHandle handle) override;

            void* mapBuffer(BufferHandle handle, MapType mapType = MapType::ReadWrite) override;
            void  unmapBuffer(BufferHandle handle) override;

            void updateTexture(TextureHandle texture, const TextureSubresourceData& data) override;
            void updateBuffer(BufferHandle buffer, const void* data, u64 offset, u64 size) override;
            void copyTexture(TextureHandle dst, TextureHandle src, const ImageCopy& copyInfo) override;
            void copyBuffer(BufferHandle dst, BufferHandle src, const BufferCopy& copyInfo) override;
            void copyBufferToTexture(TextureHandle          dst, BufferHandle src,
                                     const BufferImageCopy& copyInfo) override;
            void copyTextureToBuffer(BufferHandle           dst, TextureHandle src,
                                     const BufferImageCopy& copyInfo) override;

            void transitionResource(TextureHandle           handle, ResourceState newState,
                                    const SubresourceRange& range = {}) override;
            void transitionResource(BufferHandle handle, ResourceState newState) override;

            u64        getResourceSize(ResourceHandle handle) const override;
            MemoryType getResourceMemoryType(ResourceHandle handle) const override;
            bool       isResourceValid(ResourceHandle handle) const override;
            bool       isTextureValid(TextureHandle handle) const override;
            bool       isBufferValid(BufferHandle handle) const override;

            MemoryStatistics getMemoryStatistics() const override;
            void             dumpMemoryInfo(const char* filename) const override;

            void setResourceCacheSize(u64 maxSize) override;
            void clearResourceCache() override;

            TextureHandle acquireTransientTexture(const TextureCreateInfo& desc) override;
            void          releaseTransientTexture(TextureHandle handle) override;
            BufferHandle  acquireTransientBuffer(const BufferCreateInfo& desc) override;
            void          releaseTransientBuffer(BufferHandle handle) override;

            u64 getBufferDeviceAddress(BufferHandle handle) override;

            // Vulkan特定方法
            VulkanTexture*      getVulkanTexture(TextureHandle handle);
            VulkanBuffer*       getVulkanBuffer(BufferHandle handle);
            VulkanSampler*      getVulkanSampler(SamplerHandle handle);
            VulkanResourceView* getVulkanResourceView(ResourceViewHandle handle);

            // 格式转换
            static VkFormat              convertFormat(Format format);
            static Format                convertFormat(VkFormat format);
            static VkImageType           convertTextureType(TextureType type);
            static VkImageViewType       convertImageViewType(TextureType type);
            static VkImageUsageFlags     convertUsage(ResourceUsage usage);
            static VkImageAspectFlags    getAspectFlags(Format format);
            static VkImageLayout         convertImageLayout(ImageLayout layout);
            static VkImageLayout         convertResourceStateToLayout(ResourceState state);
            static VkBufferUsageFlags    convertBufferUsage(ResourceUsage usage);
            static VkMemoryPropertyFlags convertMemoryType(MemoryType type);
            static VkFilter              convertFilter(Filter filter);
            static VkSamplerMipmapMode   convertMipmapMode(Filter filter);
            static VkSamplerAddressMode  convertAddressMode(AddressMode mode);
            static VkCompareOp           convertComparisonFunc(ComparisonFunc func);
            static VkBorderColor         convertBorderColor(const std::array<float, 4>& color);

        private:
            VulkanDevice* device;

            // 资源存储
            std::unordered_map<TextureHandle, std::unique_ptr<VulkanTexture>>           textures;
            std::unordered_map<BufferHandle, std::unique_ptr<VulkanBuffer>>             buffers;
            std::unordered_map<SamplerHandle, std::unique_ptr<VulkanSampler>>           samplers;
            std::unordered_map<ResourceViewHandle, std::unique_ptr<VulkanResourceView>> resourceViews;

            // 句柄分配器
            HandleAllocator<TextureHandle>      textureHandleAllocator;
            HandleAllocator<BufferHandle>       bufferHandleAllocator;
            HandleAllocator<SamplerHandle>      samplerHandleAllocator;
            HandleAllocator<ResourceViewHandle> resourceViewHandleAllocator;

            // 瞬态资源池
            struct TransientTextureEntry {
                std::unique_ptr<VulkanTexture> texture;
                TextureCreateInfo              desc;
                bool                           inUse = false;
            };
            struct TransientBufferEntry {
                std::unique_ptr<VulkanBuffer> buffer;
                BufferCreateInfo              desc;
                bool                          inUse = false;
            };
            std::vector<TransientTextureEntry> transientTexturePool;
            std::vector<TransientBufferEntry>  transientBufferPool;

            // 内存统计
            MemoryStatistics memoryStats;

            // 辅助函数
            void updateMemoryStatistics();
            u32  calculateMipLevels(const ResourceExtent& extent, u32 requestedLevels) const;
    };

}
