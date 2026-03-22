#pragma once

#include "interface/IRenderSystem.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;

    /**
 * Vulkan调试接口实现
 */
    class VulkanDebug : public IRenderDebug {
        public:
            explicit VulkanDebug(VulkanDevice* device);
            ~VulkanDebug() override;

            bool initialize();
            void shutdown();

            // IRenderDebug接口实现
            void beginDebugRegion(const char* name, const Color& color) override;
            void endDebugRegion() override;
            void insertDebugMarker(const char* name, const Color& color) override;

            void setObjectName(ResourceHandle handle, const char* name) override;
            void setObjectName(TextureHandle handle, const char* name) override;
            void setObjectName(BufferHandle handle, const char* name) override;
            void setObjectName(PipelineHandle handle, const char* name) override;

            void enableDebugOutput(bool enable) override;
            void setDebugBreakSeverity(u32 severity) override;

            // Vulkan特定方法
            void setDebugObjectName(u64 objectHandle, VkObjectType objectType, const char* name);
            void beginCommandBufferLabel(VkCommandBuffer cmdBuffer, const char* name, const Color& color);
            void endCommandBufferLabel(VkCommandBuffer cmdBuffer);
            void insertCommandBufferLabel(VkCommandBuffer cmdBuffer, const char* name, const Color& color);

            // 调试回调
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT             messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*                                       pUserData);

        private:
            VulkanDevice* device;
            bool          enabled       = false;
            u32           breakSeverity = 0;

            // 对象名称映射
            std::unordered_map<u64, std::string> objectNames;

            // 辅助函数
            VkObjectType getResourceObjectType(ResourceHandle handle);
            VkObjectType getTextureObjectType();
            VkObjectType getBufferObjectType();
            VkObjectType getPipelineObjectType();
    };

    /**
 * Vulkan性能分析器实现
 */
    class VulkanProfiler : public IRenderProfiler {
        public:
            explicit VulkanProfiler(VulkanDevice* device);
            ~VulkanProfiler() override;

            bool initialize();
            void shutdown();

            // IRenderProfiler接口实现
            void   beginGpuTimestamp(const char* name) override;
            void   endGpuTimestamp(const char* name) override;
            double getGpuTimestamp(const char* name) override;

            void beginFrame() override;
            void endFrame() override;

            u32 getDrawCallCount() const override;
            u32 getTriangleCount() const override;
            u32 getDispatchCount() const override;
            u32 getBarrierCount() const override;

            void dumpProfileData(const char* filename) override;
            void reset() override;

            // Vulkan特定方法
            void registerCommandBuffer(VkCommandBuffer cmdBuffer);
            void unregisterCommandBuffer(VkCommandBuffer cmdBuffer);

            // 统计更新
            void incrementDrawCalls(u32 count = 1);
            void incrementTriangles(u32 count);
            void incrementDispatches(u32 count = 1);
            void incrementBarriers(u32 count = 1);

        private:
            VulkanDevice* device;

            // 时间戳查询池
            struct TimestampQuery {
                QueryPoolHandle pool;
                u32             queryIndex;
                bool            active = false;
            };
            std::unordered_map<std::string, TimestampQuery> timestampQueries;
            std::vector<QueryPoolHandle>                    queryPools;
            u32                                             currentQueryIndex = 0;

            // 统计
            struct FrameStats {
                u32 drawCalls  = 0;
                u32 triangles  = 0;
                u32 dispatches = 0;
                u32 barriers   = 0;
            };
            FrameStats              currentStats;
            std::vector<FrameStats> frameHistory;

            // GPU时间戳频率
            double timestampPeriod = 1.0;

            // 辅助函数
            QueryPoolHandle allocateQueryPool();
            u32             allocateQueryIndex();
            double          convertTimestampToMs(u64 timestamp);
    };

}
