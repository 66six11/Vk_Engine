#pragma once

#include "interface/IShaderManager.h"
#include "VulkanTypes.h"

namespace render::vulkan
{

    // 前向声明
    class VulkanDevice;

    /**
 * Vulkan着色器管理器实现
 */
    class VulkanShaderManager : public IShaderManager {
        public:
            explicit VulkanShaderManager(VulkanDevice* device);
            ~VulkanShaderManager() override;

            // IShaderManager接口实现
            bool initialize(const char* shaderRootPath) override;
            void shutdown() override;

            ShaderCompileResult compileShader(const char*                 source,
                                              const ShaderCompileOptions& options) override;
            ShaderCompileResult compileShaderFromFile(const char*                 filename,
                                                      const ShaderCompileOptions& options) override;

            IShaderProgram* createProgram(const ShaderProgramDesc& desc) override;
            void            destroyProgram(IShaderProgram* program) override;
            IShaderProgram* getProgram(const char* name) override;

            void setIncludeHandler(IShaderIncludeHandler* handler) override;

            void setCacheEnabled(bool enabled) override;
            void clearCache() override;
            bool loadCache(const char* filename) override;
            bool saveCache(const char* filename) override;

            void enableHotReload(bool enabled) override;
            void checkForChanges() override;
            void reloadAll() override;

            u32 getProgramCount() const override;
            u32 getCacheHitCount() const override;
            u32 getCacheMissCount() const override;

            // Vulkan特定方法
            VkShaderModule createShaderModule(const std::vector<u8>& spirv);
            void           destroyShaderModule(VkShaderModule module);

        private:
            VulkanDevice* device;

            // 着色器程序
            struct ShaderProgram :  IShaderProgram {
                std::string                                                                    name;
                ShaderProgramDesc                                                              desc;
                std::unordered_map<ShaderStage, ShaderHandle>                                  shaders;
                std::unordered_map<std::string, std::unordered_map<ShaderStage, ShaderHandle>> variantShaders;

                const char*              getName() const override { return name.c_str(); }
                const ShaderProgramDesc& getDesc() const override { return desc; }
                ShaderHandle             getShader(ShaderStage stage) const override;
                ShaderHandle             getShader(ShaderStage stage, const char* variant) const override;
                std::vector<std::string> getVariantNames() const override;
                bool                     reload() override;
            };

            std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> programs;
            std::unordered_map<VkShaderModule, std::vector<u8>>             shaderModules;

            // 包含处理器
            IShaderIncludeHandler*                    includeHandler = nullptr;
            std::unique_ptr<FileShaderIncludeHandler> defaultIncludeHandler;

            // 缓存
            bool                                                                        cacheEnabled = true;
            std::unordered_map<ShaderCacheKey, ShaderCompileResult, ShaderCacheKeyHash> cache;
            u32                                                                         cacheHits   = 0;
            u32                                                                         cacheMisses = 0;

            // 热重载
            bool                                                             hotReloadEnabled = false;
            std::unordered_map<std::string, std::filesystem::file_time_type> fileTimestamps;

            // 辅助函数
            std::vector<u8> compileToSpirv(const char*                 source,
                                           const ShaderCompileOptions& options);
            std::vector<u8> compileHlslToSpirv(const char*                 source,
                                               const ShaderCompileOptions& options);
            std::vector<u8> compileGlslToSpirv(const char*                 source,
                                               const ShaderCompileOptions& options);
            ShaderCacheKey computeCacheKey(const char*                 source,
                                           const ShaderCompileOptions& options);
            void processIncludes(std::string& source, const char* filename);
            bool hasFileChanged(const char* filename);
    };

}
