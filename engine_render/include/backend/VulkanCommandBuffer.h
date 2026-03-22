#pragma once

#include "interface/ICommandBuffer.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;
    class VulkanCommandAllocator;

    /**
 * Vulkan命令缓冲区实现
 */
    class VulkanCommandBuffer : public ICommandBuffer {
        public:
            VulkanCommandBuffer(VulkanDevice*   device, VulkanCommandAllocator*   allocator,
                                VkCommandBuffer commandBuffer, CommandBufferLevel level);
            ~VulkanCommandBuffer() override;

            // ICommandBuffer接口实现
            void begin() override;
            void beginSecondary(const RenderPassBeginInfo& renderPassInfo) override;
            void end() override;
            void reset() override;

            bool               isRecording() const override;
            CommandBufferLevel getLevel() const override;

            void bindPipeline(PipelineHandle pipeline) override;

            void bindDescriptorSet(u32                     setIndex, DescriptorSetHandle descriptorSet,
                                   const std::vector<u32>& dynamicOffsets = {}) override;
            void bindDescriptorSets(u32                     firstSet, const std::vector<DescriptorSetHandle>& sets,
                                    const std::vector<u32>& dynamicOffsets = {}) override;

            void bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset = 0) override;
            void bindVertexBuffers(u32                     firstBinding, const std::vector<BufferHandle>& buffers,
                                   const std::vector<u64>& offsets) override;
            void bindIndexBuffer(BufferHandle buffer, IndexType indexType, u64 offset = 0) override;

            void pushConstants(PipelineLayoutHandle layout, ShaderStage stages,
                               u32                  offset, u32         size, const void* data) override;

            void setViewport(const Viewport& viewport) override;
            void setViewports(const std::vector<Viewport>& viewports) override;
            void setScissor(const Rect& scissor) override;
            void setScissors(const std::vector<Rect>& scissors) override;
            void setLineWidth(float width) override;
            void setDepthBias(float constantFactor, float clamp, float slopeFactor) override;
            void setBlendConstants(const std::array<float, 4>& constants) override;
            void setDepthBounds(float minBounds, float maxBounds) override;
            void setStencilReference(u8 reference) override;

            void draw(u32 vertexCount, u32 instanceCount = 1,
                      u32 firstVertex                    = 0, u32 firstInstance = 0) override;
            void drawIndexed(u32 indexCount, u32 instanceCount = 1,
                             u32 firstIndex                    = 0, i32 vertexOffset = 0,
                             u32 firstInstance                 = 0) override;
            void drawIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) override;
            void drawIndexedIndirect(BufferHandle buffer, u64 offset, u32 drawCount, u32 stride) override;

            void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) override;
            void dispatchIndirect(BufferHandle buffer, u64 offset) override;

            void copyBuffer(BufferHandle src, BufferHandle dst, const BufferCopy& region) override;
            void copyImage(TextureHandle src, TextureHandle dst, const ImageCopy& region) override;
            void copyBufferToImage(BufferHandle           src, TextureHandle dst,
                                   const BufferImageCopy& region) override;
            void copyImageToBuffer(TextureHandle          src, BufferHandle dst,
                                   const BufferImageCopy& region) override;
            void blitImage(TextureHandle                 src, TextureHandle dst,
                           const std::vector<ImageCopy>& regions, Filter    filter = Filter::Linear) override;
            void resolveImage(TextureHandle                 src, TextureHandle dst,
                              const std::vector<ImageCopy>& regions) override;

            void clearColorImage(TextureHandle                        image, const ClearValue& color,
                                 const std::vector<SubresourceRange>& ranges) override;
            void clearDepthStencilImage(TextureHandle                        image, const ClearValue& depthStencil,
                                        const std::vector<SubresourceRange>& ranges) override;
            void fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) override;
            void updateBuffer(BufferHandle buffer, u64 offset, u64 size, const void* data) override;

            void pipelineBarrier(PipelineStage                       srcStage, PipelineStage dstStage,
                                 const std::vector<ResourceBarrier>& barriers) override;
            void memoryBarrier(PipelineStage srcStage, PipelineStage dstStage,
                               AccessFlags   srcAccess, AccessFlags  dstAccess) override;
            void bufferMemoryBarrier(BufferHandle  buffer, u64             offset, u64 size,
                                     PipelineStage srcStage, PipelineStage dstStage,
                                     AccessFlags   srcAccess, AccessFlags  dstAccess) override;
            void imageMemoryBarrier(TextureHandle           texture,
                                    ImageLayout             oldLayout, ImageLayout  newLayout,
                                    PipelineStage           srcStage, PipelineStage dstStage,
                                    AccessFlags             srcAccess, AccessFlags  dstAccess,
                                    const SubresourceRange& range = {}) override;

            void beginRenderPass(const RenderPassBeginInfo& beginInfo) override;
            void endRenderPass() override;
            void nextSubpass() override;

            void resetQueryPool(QueryPoolHandle pool, u32 firstQuery, u32 queryCount) override;
            void beginQuery(QueryPoolHandle pool, u32 query) override;
            void endQuery(QueryPoolHandle pool, u32 query) override;
            void writeTimestamp(PipelineStage stage, QueryPoolHandle pool, u32 query) override;
            void copyQueryPoolResults(QueryPoolHandle pool, u32      firstQuery, u32 queryCount,
                                      BufferHandle    dstBuffer, u64 dstOffset, u64  stride) override;

            void beginDebugMarker(const char* name, const Color& color) override;
            void endDebugMarker() override;
            void insertDebugMarker(const char* name, const Color& color) override;

            void executeCommands(const std::vector<ICommandBuffer*>& commandBuffers) override;

            void buildAccelerationStructures() override;
            void traceRays() override;

            u32 getCommandCount() const override;

            // Vulkan特定方法
            VkCommandBuffer getVkCommandBuffer() const { return commandBuffer; }
            VulkanDevice*   getDevice() const { return device; }

        private:
            VulkanDevice*           device;
            VulkanCommandAllocator* allocator;
            VkCommandBuffer         commandBuffer;
            CommandBufferLevel      level;

            bool recording    = false;
            u32  commandCount = 0;

            // 当前绑定状态（用于优化）
            PipelineHandle                   currentPipeline;
            std::vector<DescriptorSetHandle> currentDescriptorSets;
            BufferHandle                     currentIndexBuffer;
            BufferHandle                     currentVertexBuffer;

            // 辅助函数
            VkPipelineBindPoint getPipelineBindPoint(PipelineHandle pipeline) const;
            VkIndexType         convertIndexType(IndexType type) const;
            VkRenderPass        getOrCreateRenderPass(const RenderPassBeginInfo& beginInfo);
            VkFramebuffer       getOrCreateFramebuffer(const RenderPassBeginInfo& beginInfo);
    };

}
