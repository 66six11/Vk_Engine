#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class IRenderDevice;
class IHeap;

/**
 * 纹理创建信息
 */
struct TextureCreateInfo {
    ResourceExtent extent = {1, 1, 1};
    Format format = Format::RGBA8_UNORM;
    u32 mipLevels = 1;
    u32 arrayLayers = 1;
    TextureType type = TextureType::Texture2D;
    ResourceUsage usage = ResourceUsage::ShaderResource;
    MemoryType memoryType = MemoryType::Default;
    ClearValue clearValue = {};
    SamplerState samplerState = {};
    std::string debugName;

    // 计算mip级别
    u32 computeMipLevels() const {
        if (mipLevels > 1) return mipLevels;
        u32 maxDim = std::max({extent.width, extent.height, extent.depth});
        return static_cast<u32>(std::floor(std::log2(maxDim))) + 1;
    }
};

/**
 * 缓冲区创建信息
 */
struct BufferCreateInfo {
    u64 size = 0;
    BufferType type = BufferType::Default;
    ResourceUsage usage = ResourceUsage::VertexBuffer;
    MemoryType memoryType = MemoryType::Upload;
    std::optional<u32> stride; // 结构化缓冲区
    bool deviceAddress = false;
    std::string debugName;
};

/**
 * 资源视图描述
 */
struct ResourceViewDesc {
    ResourceViewType type = ResourceViewType::ShaderResource;
    Format format = Format::Unknown;
    u32 baseMipLevel = 0;
    u32 mipLevelCount = 1;
    u32 baseArrayLayer = 0;
    u32 arrayLayerCount = 1;
    ComponentMapping components = {};
};

/**
 * 纹理视图描述
 */
struct TextureViewDesc : ResourceViewDesc {
    TextureViewDesc() {
        type = ResourceViewType::ShaderResource;
    }
};

/**
 * 缓冲区视图描述
 */
struct BufferViewDesc : ResourceViewDesc {
    u64 offset = 0;
    u64 range = 0; // 0表示全部

    BufferViewDesc() {
        type = ResourceViewType::ShaderResource;
    }
};

/**
 * 纹理子资源数据
 */
struct TextureSubresourceData {
    const void* data = nullptr;
    u32 rowPitch = 0;
    u32 slicePitch = 0;
    u32 mipLevel = 0;
    u32 arrayLayer = 0;
};

/**
 * 采样器描述
 */
struct SamplerDesc {
    Filter minFilter = Filter::Linear;
    Filter magFilter = Filter::Linear;
    Filter mipFilter = Filter::Linear;
    AddressMode addressU = AddressMode::Wrap;
    AddressMode addressV = AddressMode::Wrap;
    AddressMode addressW = AddressMode::Wrap;
    float mipLODBias = 0.0f;
    float maxAnisotropy = 1.0f;
    ComparisonFunc comparisonFunc = ComparisonFunc::Never;
    float minLOD = 0.0f;
    float maxLOD = 3.402823466e+38f;
    std::array<float, 4> borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
    bool unnormalizedCoordinates = false;
};

/**
 * 纹理信息
 */
struct TextureInfo {
    ResourceExtent extent;
    Format format;
    u32 mipLevels;
    u32 arrayLayers;
    TextureType type;
    ResourceUsage usage;
    MemoryType memoryType;
    u64 size;
};

/**
 * 缓冲区信息
 */
struct BufferInfo {
    u64 size;
    BufferType type;
    ResourceUsage usage;
    MemoryType memoryType;
    std::optional<u32> stride;
    bool deviceAddress;
};

/**
 * 资源管理器接口
 * 负责纹理、缓冲区、采样器等GPU资源的生命周期管理
 */
class IResourceManager {
public:
    virtual ~IResourceManager() = default;

    // 纹理管理
    virtual TextureHandle createTexture(const TextureCreateInfo& createInfo) = 0;
    virtual TextureHandle createTextureFromData(const void* data,
                                               const TextureCreateInfo& createInfo,
                                               u32 rowPitch = 0) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual TextureInfo getTextureInfo(TextureHandle handle) const = 0;

    // 缓冲区管理
    virtual BufferHandle createBuffer(const BufferCreateInfo& createInfo) = 0;
    virtual BufferHandle createBufferFromData(const void* data, u64 size,
                                            const BufferCreateInfo& createInfo) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual BufferInfo getBufferInfo(BufferHandle handle) const = 0;

    // 采样器管理
    virtual SamplerHandle createSampler(const SamplerDesc& desc) = 0;
    virtual void destroySampler(SamplerHandle handle) = 0;

    // 资源视图
    virtual ResourceViewHandle createTextureView(TextureHandle texture,
                                                const TextureViewDesc& desc = {}) = 0;
    virtual ResourceViewHandle createBufferView(BufferHandle buffer,
                                               const BufferViewDesc& desc = {}) = 0;
    virtual void destroyResourceView(ResourceViewHandle handle) = 0;

    // 内存管理
    virtual void* mapBuffer(BufferHandle handle,
                           MapType mapType = MapType::ReadWrite) = 0;
    virtual void unmapBuffer(BufferHandle handle) = 0;

    // 更新与复制
    virtual void updateTexture(TextureHandle texture,
                              const TextureSubresourceData& data) = 0;
    virtual void updateBuffer(BufferHandle buffer, const void* data,
                             u64 offset, u64 size) = 0;
    virtual void copyTexture(TextureHandle dst, TextureHandle src,
                            const ImageCopy& copyInfo) = 0;
    virtual void copyBuffer(BufferHandle dst, BufferHandle src,
                           const BufferCopy& copyInfo) = 0;
    virtual void copyBufferToTexture(TextureHandle dst, BufferHandle src,
                                    const BufferImageCopy& copyInfo) = 0;
    virtual void copyTextureToBuffer(BufferHandle dst, TextureHandle src,
                                    const BufferImageCopy& copyInfo) = 0;

    // 资源状态管理（由Render Graph自动处理，通常不需要手动调用）
    virtual void transitionResource(TextureHandle handle,
                                   ResourceState newState,
                                   const SubresourceRange& range = {}) = 0;
    virtual void transitionResource(BufferHandle handle,
                                   ResourceState newState) = 0;

    // 查询功能
    virtual u64 getResourceSize(ResourceHandle handle) const = 0;
    virtual MemoryType getResourceMemoryType(ResourceHandle handle) const = 0;
    virtual bool isResourceValid(ResourceHandle handle) const = 0;
    virtual bool isTextureValid(TextureHandle handle) const = 0;
    virtual bool isBufferValid(BufferHandle handle) const = 0;

    // 内存统计
    virtual MemoryStatistics getMemoryStatistics() const = 0;
    virtual void dumpMemoryInfo(const char* filename) const = 0;

    // 缓存管理
    virtual void setResourceCacheSize(u64 maxSize) = 0;
    virtual void clearResourceCache() = 0;

    // 瞬态资源（Transient Resources）
    virtual TextureHandle acquireTransientTexture(const TextureCreateInfo& desc) = 0;
    virtual void releaseTransientTexture(TextureHandle handle) = 0;
    virtual BufferHandle acquireTransientBuffer(const BufferCreateInfo& desc) = 0;
    virtual void releaseTransientBuffer(BufferHandle handle) = 0;

    // 设备地址（用于光追）
    virtual u64 getBufferDeviceAddress(BufferHandle handle) = 0;
};

/**
 * 资源上传上下文
 * 用于批量上传资源到GPU
 */
class IUploadContext {
public:
    virtual ~IUploadContext() = default;

    // 上传纹理
    virtual void uploadTexture(TextureHandle texture,
                              const void* data,
                              u32 rowPitch = 0,
                              u32 slicePitch = 0) = 0;

    // 上传缓冲区
    virtual void uploadBuffer(BufferHandle buffer,
                             const void* data,
                             u64 offset = 0,
                             u64 size = 0) = 0;

    // 生成mipmap
    virtual void generateMipmaps(TextureHandle texture) = 0;

    // 提交上传
    virtual void submit() = 0;
    virtual void submitAndWait() = 0;
};

} // namespace render