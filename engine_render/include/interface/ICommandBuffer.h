#pragma once

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class IRenderDevice;
class ICommandQueue;

/**
 * @brief 资源屏障类型
 * @details 不同类型的GPU资源屏障
 */
enum class BarrierType {
    Global,     ///< 全局内存屏障
    Buffer,     ///< 缓冲区屏障
    Image,      ///< 图像/纹理屏障
    Aliasing,   ///< 资源别名屏障
};

/**
 * 资源屏障
 */
struct ResourceBarrier {
    BarrierType type = BarrierType::Global;

    // 全局屏障
    PipelineStage srcStage = PipelineStage::TopOfPipe;
    PipelineStage dstStage = PipelineStage::BottomOfPipe;
    AccessFlags srcAccess = AccessFlags::None;
    AccessFlags dstAccess = AccessFlags::None;

    // 缓冲区屏障
    BufferHandle buffer;
    u64 offset = 0;
    u64 size = 0;

    // 图像屏障
    TextureHandle beforeTexture;
    TextureHandle afterTexture;
    ImageLayout oldLayout = ImageLayout::Undefined;
    ImageLayout newLayout = ImageLayout::General;
    SubresourceRange subresourceRange;

    // 队列家族所有权转移
    u32 srcQueueFamily = ~0u;
    u32 dstQueueFamily = ~0u;

    // 构建全局屏障
    static ResourceBarrier global(PipelineStage srcStage, PipelineStage dstStage,
                                 AccessFlags srcAccess = AccessFlags::None,
                                 AccessFlags dstAccess = AccessFlags::None) {
        ResourceBarrier barrier;
        barrier.type = BarrierType::Global;
        barrier.srcStage = srcStage;
        barrier.dstStage = dstStage;
        barrier.srcAccess = srcAccess;
        barrier.dstAccess = dstAccess;
        return barrier;
    }

    // 构建缓冲区屏障
    static ResourceBarrier bufferBarrier(BufferHandle bufferHandle,
                                 PipelineStage srcStage, PipelineStage dstStage,
                                 AccessFlags srcAccess, AccessFlags dstAccess,
                                 u64 offset = 0, u64 size = ~0ull) {
        ResourceBarrier barrier;
        barrier.type = BarrierType::Buffer;
        barrier.buffer = bufferHandle;
        barrier.srcStage = srcStage;
        barrier.dstStage = dstStage;
        barrier.srcAccess = srcAccess;
        barrier.dstAccess = dstAccess;
        barrier.offset = offset;
        barrier.size = size;
        return barrier;
    }

    // 构建图像屏障
    static ResourceBarrier image(TextureHandle texture,
                                ImageLayout oldLayout, ImageLayout newLayout,
                                PipelineStage srcStage, PipelineStage dstStage,
                                AccessFlags srcAccess, AccessFlags dstAccess,
                                const SubresourceRange& range = {}) {
        ResourceBarrier barrier;
        barrier.type = BarrierType::Image;
        barrier.afterTexture = texture;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcStage = srcStage;
        barrier.dstStage = dstStage;
        barrier.srcAccess = srcAccess;
        barrier.dstAccess = dstAccess;
        barrier.subresourceRange = range;
        return barrier;
    }

    // 构建别名屏障
    static ResourceBarrier aliasing(TextureHandle beforeTexture,
                                   TextureHandle afterTexture) {
        ResourceBarrier barrier;
        barrier.type = BarrierType::Aliasing;
        barrier.beforeTexture = beforeTexture;
        barrier.afterTexture = afterTexture;
        // 别名屏障使用特殊的布局和阶段
        barrier.oldLayout = ImageLayout::Undefined;
        barrier.newLayout = ImageLayout::General;
        barrier.srcStage = PipelineStage::TopOfPipe;
        barrier.dstStage = PipelineStage::BottomOfPipe;
        return barrier;
    }
};

/**
 * 渲染通道开始信息 
 */
struct RenderPassBeginInfo {
    RenderPassHandle renderPass;
    FramebufferHandle framebuffer;
    Rect renderArea;
    std::vector<ClearValue> clearValues;
};

/**
 * 命令缓冲区接口
 * 用于录制GPU命令
 */
class ICommandBuffer {
public:
    virtual ~ICommandBuffer() = default;

    // 录制控制
    virtual void begin() = 0;
    virtual void beginSecondary(const RenderPassBeginInfo& renderPassInfo) = 0;
    virtual void end() = 0;
    virtual void reset() = 0;

    // 查询状态
    virtual bool isRecording() const = 0;
    virtual CommandBufferLevel getLevel() const = 0;

    // 管线绑定
    virtual void bindPipeline(PipelineHandle pipeline) = 0;

    // 描述符集绑定
    virtual void bindDescriptorSet(u32 setIndex, DescriptorSetHandle descriptorSet,
                                  const std::vector<u32>& dynamicOffsets = {}) = 0;
    virtual void bindDescriptorSets(u32 firstSet,
                                   const std::vector<DescriptorSetHandle>& sets,
                                   const std::vector<u32>& dynamicOffsets = {}) = 0;

    // 顶点/索引缓冲区绑定
    virtual void bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset = 0) = 0;
    virtual void bindVertexBuffers(u32 firstBinding,
                                  const std::vector<BufferHandle>& buffers,
                                  const std::vector<u64>& offsets) = 0;
    virtual void bindIndexBuffer(BufferHandle buffer, IndexType indexType, u64 offset = 0) = 0;

    // 推送常量
    virtual void pushConstants(PipelineLayoutHandle layout, ShaderStage stages,
                              u32 offset, u32 size, const void* data) = 0;

    // 视口和裁剪
    virtual void setViewport(const Viewport& viewport) = 0;
    virtual void setViewports(const std::vector<Viewport>& viewports) = 0;
    virtual void setScissor(const Rect& scissor) = 0;
    virtual void setScissors(const std::vector<Rect>& scissors) = 0;
    virtual void setLineWidth(float width) = 0;
    virtual void setDepthBias(float constantFactor, float clamp, float slopeFactor) = 0;
    virtual void setBlendConstants(const std::array<float, 4>& constants) = 0;
    virtual void setDepthBounds(float minBounds, float maxBounds) = 0;
    virtual void setStencilReference(u8 reference) = 0;

    // 绘制命令
    virtual void draw(u32 vertexCount, u32 instanceCount = 1,
                     u32 firstVertex = 0, u32 firstInstance = 0) = 0;
    virtual void drawIndexed(u32 indexCount, u32 instanceCount = 1,
                            u32 firstIndex = 0, i32 vertexOffset = 0,
                            u32 firstInstance = 0) = 0;
    virtual void drawIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) = 0;
    virtual void drawIndexedIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) = 0;

    // 计算命令
    virtual void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) = 0;
    virtual void dispatchIndirect(BufferHandle buffer, u64 offset) = 0;

    // 复制命令
    virtual void copyBuffer(BufferHandle src, BufferHandle dst,
                           const BufferCopy& region) = 0;
    virtual void copyImage(TextureHandle src, TextureHandle dst,
                          const ImageCopy& region) = 0;
    virtual void copyBufferToImage(BufferHandle src, TextureHandle dst,
                                  const BufferImageCopy& region) = 0;
    virtual void copyImageToBuffer(TextureHandle src, BufferHandle dst,
                                  const BufferImageCopy& region) = 0;
    virtual void blitImage(TextureHandle src, TextureHandle dst,
                          const std::vector<ImageCopy>& regions,
                          Filter filter = Filter::Linear) = 0;
    virtual void resolveImage(TextureHandle src, TextureHandle dst,
                             const std::vector<ImageCopy>& regions) = 0;

    // 清除命令
    virtual void clearColorImage(TextureHandle image, const ClearValue& color,
                                const std::vector<SubresourceRange>& ranges) = 0;
    virtual void clearDepthStencilImage(TextureHandle image, const ClearValue& depthStencil,
                                       const std::vector<SubresourceRange>& ranges) = 0;
    virtual void fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) = 0;
    virtual void updateBuffer(BufferHandle buffer, u64 offset, u64 size, const void* data) = 0;

    // 资源屏障
    virtual void pipelineBarrier(PipelineStage srcStage, PipelineStage dstStage,
                                const std::vector<ResourceBarrier>& barriers) = 0;
    virtual void memoryBarrier(PipelineStage srcStage, PipelineStage dstStage,
                              AccessFlags srcAccess, AccessFlags dstAccess) = 0;
    virtual void bufferMemoryBarrier(BufferHandle buffer, u64 offset, u64 size,
                                    PipelineStage srcStage, PipelineStage dstStage,
                                    AccessFlags srcAccess, AccessFlags dstAccess) = 0;
    virtual void imageMemoryBarrier(TextureHandle texture,
                                   ImageLayout oldLayout, ImageLayout newLayout,
                                   PipelineStage srcStage, PipelineStage dstStage,
                                   AccessFlags srcAccess, AccessFlags dstAccess,
                                   const SubresourceRange& range = {}) = 0;

    // Split Barrier支持
    // Split Barrier允许将屏障分为Begin和End两个阶段，在中间执行其他工作
    // 适用于需要在长时间计算期间提前释放资源的场景
    virtual void beginSplitBarrier(const std::vector<ResourceBarrier>& barriers,
                                   PipelineStage srcStage) = 0;
    virtual void endSplitBarrier(const std::vector<ResourceBarrier>& barriers,
                                 PipelineStage dstStage) = 0;

    // 别名屏障
    virtual void aliasingBarrier(TextureHandle beforeTexture,
                                 TextureHandle afterTexture) = 0;

    // 渲染通道（传统）
    virtual void beginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
    virtual void endRenderPass() = 0;
    virtual void nextSubpass() = 0;

    // 查询
    virtual void resetQueryPool(QueryPoolHandle pool, u32 firstQuery, u32 queryCount) = 0;
    virtual void beginQuery(QueryPoolHandle pool, u32 query) = 0;
    virtual void endQuery(QueryPoolHandle pool, u32 query) = 0;
    virtual void writeTimestamp(PipelineStage stage, QueryPoolHandle pool, u32 query) = 0;
    virtual void copyQueryPoolResults(QueryPoolHandle pool, u32 firstQuery, u32 queryCount,
                                     BufferHandle dstBuffer, u64 dstOffset, u64 stride) = 0;

    // 调试标记
    virtual void beginDebugMarker(const char* name, const Color& color) = 0;
    virtual void endDebugMarker() = 0;
    virtual void insertDebugMarker(const char* name, const Color& color) = 0;

    // 执行次级命令缓冲区
    virtual void executeCommands(const std::vector<ICommandBuffer*>& commandBuffers) = 0;

    // 光追命令（可选）
    virtual void buildAccelerationStructures() = 0;
    virtual void traceRays() = 0;

    // 统计
    virtual u32 getCommandCount() const = 0;
};

} // namespace render