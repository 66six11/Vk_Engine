#pragma once

#include "interface/IPipelineManager.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;

    /**
 * Vulkan着色器实现
 */
    class VulkanShader {
        public:
            VulkanShader(VulkanDevice* device);
            ~VulkanShader();

            bool create(const ShaderCreateInfo& createInfo);
            void destroy();

            VkShaderModule                  getModule() const { return module; }
            VkPipelineShaderStageCreateInfo getStageInfo() const;

            const ShaderInfo& getInfo() const { return info; }
            ShaderHandle      getHandle() const { return handle; }

        private:
            VulkanDevice*         device;
            VkShaderModule        module = nullptr;
            ShaderInfo            info;
            ShaderHandle          handle;
            VkShaderStageFlagBits stage;
    };

    /**
 * Vulkan管线布局实现
 */
    class VulkanPipelineLayout {
        public:
            VulkanPipelineLayout(VulkanDevice* device);
            ~VulkanPipelineLayout();

            bool create(const PipelineLayoutDesc& desc);
            void destroy();

            VkPipelineLayout          getLayout() const { return layout; }
            const PipelineLayoutDesc& getDesc() const { return desc; }

            // 推送常量范围
            const std::vector<VkPushConstantRange>& getPushConstantRanges() const {
                return pushConstantRanges;
            }

        private:
            VulkanDevice*                    device;
            VkPipelineLayout                 layout = nullptr;
            PipelineLayoutDesc               desc;
            std::vector<VkPushConstantRange> pushConstantRanges;
    };

    /**
 * Vulkan管线状态对象实现
 */
    class VulkanPipeline {
        public:
            VulkanPipeline(VulkanDevice* device);
            ~VulkanPipeline();

            // 图形管线
            bool createGraphics(const GraphicsPipelineDesc& desc);

            // 计算管线
            bool createCompute(const ComputePipelineDesc& desc);

            // 光追管线
            bool createRayTracing(const RayTracingPipelineDesc& desc);

            void destroy();

            VkPipeline           getPipeline() const { return pipeline; }
            VkPipelineBindPoint  getBindPoint() const { return bindPoint; }
            PipelineLayoutHandle getLayout() const { return layout; }
            const PipelineInfo&  getInfo() const { return info; }

            // 光追特定
            const VkStridedDeviceAddressRegionKHR* getRayGenShaderBindingTable() const;
            const VkStridedDeviceAddressRegionKHR* getMissShaderBindingTable() const;
            const VkStridedDeviceAddressRegionKHR* getHitShaderBindingTable() const;
            const VkStridedDeviceAddressRegionKHR* getCallableShaderBindingTable() const;

        private:
            VulkanDevice*        device;
            VkPipeline           pipeline  = nullptr;
            VkPipelineBindPoint  bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            PipelineLayoutHandle layout;
            PipelineInfo         info;

            // 光追着色器绑定表
            struct ShaderBindingTable {
                VkStridedDeviceAddressRegionKHR rayGen;
                VkStridedDeviceAddressRegionKHR miss;
                VkStridedDeviceAddressRegionKHR hit;
                VkStridedDeviceAddressRegionKHR callable;
            };
            std::optional<ShaderBindingTable> shaderBindingTable;

            // 辅助函数
            bool createGraphicsPipelineInternal(const GraphicsPipelineDesc& desc);
            bool createComputePipelineInternal(const ComputePipelineDesc& desc);
            bool createRayTracingPipelineInternal(const RayTracingPipelineDesc& desc);

            // 状态转换
            static VkPrimitiveTopology   convertTopology(PrimitiveTopology topology);
            static VkPolygonMode         convertFillMode(FillMode mode);
            static VkCullModeFlags       convertCullMode(CullMode mode);
            static VkFrontFace           convertFrontFace(bool counterClockwise);
            static VkCompareOp           convertComparisonFunc(ComparisonFunc func);
            static VkStencilOp           convertStencilOp(StencilOp op);
            static VkBlendFactor         convertBlendFactor(BlendFactor factor);
            static VkBlendOp             convertBlendOp(BlendOp op);
            static VkVertexInputRate     convertInputRate(VertexInputRate rate);
            static VkFormat              convertFormat(Format format);
            static VkSampleCountFlagBits convertSampleCount(u32 count);
    };

    /**
 * Vulkan管线管理器实现
 */
    class VulkanPipelineManager : public IPipelineManager {
        public:
            explicit VulkanPipelineManager(VulkanDevice* device);
            ~VulkanPipelineManager() override;

            bool initialize();
            void shutdown();

            // IPipelineManager接口实现
            ShaderHandle createShader(const ShaderCreateInfo& createInfo) override;
            void         destroyShader(ShaderHandle handle) override;
            ShaderInfo   getShaderInfo(ShaderHandle handle) const override;
            ShaderHandle getShaderByName(const char* name) const override;

            PipelineLayoutHandle createPipelineLayout(const PipelineLayoutDesc& desc) override;
            void                 destroyPipelineLayout(PipelineLayoutHandle handle) override;

            PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
            PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc,
                                                  const char*                 name) override;
            PipelineHandle createComputePipeline(const ComputePipelineDesc& desc) override;
            PipelineHandle createComputePipeline(const ComputePipelineDesc& desc,
                                                 const char*                name) override;
            PipelineHandle createRayTracingPipeline(const RayTracingPipelineDesc& desc) override;
            void           destroyPipeline(PipelineHandle handle) override;

            PipelineInfo   getPipelineInfo(PipelineHandle handle) const override;
            PipelineHandle getPipelineByName(const char* name) const override;

            bool reloadShader(ShaderHandle handle, const void* data, size_t size) override;
            bool reloadPipeline(PipelineHandle handle) override;
            void enableHotReload(bool enable) override;
            void checkForChanges() override;

            void            setPipelineCacheSize(size_t maxSize) override;
            void            clearPipelineCache() override;
            bool            loadPipelineCache(const void* data, size_t size) override;
            std::vector<u8> savePipelineCache() override;

            void compileAsync(PipelineHandle handle) override;
            bool isCompilationComplete(PipelineHandle handle) override;
            void waitForCompilation() override;

            void dumpPipelineInfo(PipelineHandle handle, const char* filename) override;
            void dumpAllPipelines(const char* filename) override;

            // Vulkan特定方法
            VulkanShader*         getVulkanShader(ShaderHandle handle);
            VulkanPipeline*       getVulkanPipeline(PipelineHandle handle);
            VulkanPipelineLayout* getVulkanPipelineLayout(PipelineLayoutHandle handle);

            VkPipelineCache getPipelineCache() const { return pipelineCache; }

        private:
            VulkanDevice*   device;
            VkPipelineCache pipelineCache = nullptr;

            // 资源存储
            std::unordered_map<ShaderHandle, std::unique_ptr<VulkanShader>>                 shaders;
            std::unordered_map<PipelineHandle, std::unique_ptr<VulkanPipeline>>             pipelines;
            std::unordered_map<PipelineLayoutHandle, std::unique_ptr<VulkanPipelineLayout>> layouts;

            // 句柄分配器
            HandleAllocator<ShaderHandle>         shaderHandleAllocator;
            HandleAllocator<PipelineHandle>       pipelineHandleAllocator;
            HandleAllocator<PipelineLayoutHandle> layoutHandleAllocator;

            // 命名查找
            std::unordered_map<std::string, ShaderHandle>   namedShaders;
            std::unordered_map<std::string, PipelineHandle> namedPipelines;

            // 热重载
            bool                                                              hotReloadEnabled = false;
            std::unordered_map<ShaderHandle, std::string>                     shaderFilePaths;
            std::unordered_map<ShaderHandle, std::filesystem::file_time_type> shaderLastModified;

            // 异步编译
            struct AsyncCompileTask {
                PipelineHandle    handle;
                std::future<bool> future;
            };
            std::vector<AsyncCompileTask> asyncTasks;

            // 缓存
            size_t maxCacheSize = 64 * 1024 * 1024; // 64MB

            // 辅助函数
            void updateShaderModificationTime(ShaderHandle handle);
            bool hasShaderChanged(ShaderHandle handle);
            void reloadChangedShaders();
    };

}
