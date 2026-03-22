#pragma once

#include <string>
#include <vector>

#include "core/Types.h"
#include "core/Handle.h"

namespace render {

// 前向声明
class IPipelineManager;

/**
 * @brief 着色器语言
 * @details 支持的源着色器语言
 */
enum class ShaderLanguage {
    HLSL,       ///< DirectX高级着色语言
    GLSL,       ///< OpenGL着色语言
    SPIRV,      ///< 标准可移植中间表示
    MSL,        ///< Metal着色语言
    WGSL,       ///< WebGPU着色语言
    Count       ///< 语言数量
};

/**
 * @brief 着色器编译目标
 * @details 着色器编译输出的目标格式版本
 */
enum class ShaderTarget {
    SPIRV_1_0,  ///< SPIR-V 1.0
    SPIRV_1_3,  ///< SPIR-V 1.3
    SPIRV_1_5,  ///< SPIR-V 1.5
    SPIRV_1_6,  ///< SPIR-V 1.6
    DXIL_6_0,   ///< DXIL 6.0
    DXIL_6_5,   ///< DXIL 6.5
    DXIL_6_6,   ///< DXIL 6.6
    MSL_2_0,    ///< Metal Shading Language 2.0
    MSL_2_3,    ///< Metal Shading Language 2.3
    Count       ///< 目标数量
};

/**
 * 着色器编译选项
 */
struct ShaderCompileOptions {
    ShaderLanguage sourceLanguage = ShaderLanguage::HLSL;
    ShaderTarget target = ShaderTarget::SPIRV_1_5;
    ShaderStage stage = ShaderStage::Vertex;
    std::string entryPoint = "main";

    // 宏定义
    std::vector<std::pair<std::string, std::string>> defines;

    // 优化级别 (0-3)
    u32 optimizationLevel = 2;

    // 调试信息
    bool generateDebugInfo = false;
    bool generateSourceMap = false;

    // 其他选项
    bool warningsAsErrors = false;
    bool strictMode = false;
};

/**
 * 着色器编译结果
 */
struct ShaderCompileResult {
    bool success = false;
    std::vector<u8> bytecode;
    std::string errorMessage;
    std::vector<std::string> warnings;

    // 反射信息
    struct ReflectionInfo {
        struct ResourceBinding {
            std::string name;
            u32 binding;
            u32 set;
            DescriptorType type;
            ShaderStage stages;
        };
        std::vector<ResourceBinding> bindings;

        struct PushConstant {
            std::string name;
            u32 offset;
            u32 size;
        };
        std::vector<PushConstant> pushConstants;

        struct InputOutput {
            std::string name;
            u32 location;
            Format format;
        };
        std::vector<InputOutput> inputs;
        std::vector<InputOutput> outputs;

        u32 threadGroupSizeX = 1;
        u32 threadGroupSizeY = 1;
        u32 threadGroupSizeZ = 1;
    };
    std::optional<ReflectionInfo> reflection;
};

/**
 * 着色器缓存键
 */
struct ShaderCacheKey {
    std::string sourceHash;
    std::string optionsHash;

    bool operator==(const ShaderCacheKey& other) const {
        return sourceHash == other.sourceHash && optionsHash == other.optionsHash;
    }
};

struct ShaderCacheKeyHash {
    size_t operator()(const ShaderCacheKey& key) const {
        return std::hash<std::string>()(key.sourceHash) ^
               (std::hash<std::string>()(key.optionsHash) << 1);
    }
};

/**
 * 着色器变体
 */
struct ShaderVariant {
    std::string name;
    std::vector<std::pair<std::string, std::string>> defines;
};

/**
 * 着色器程序描述
 */
struct ShaderProgramDesc {
    std::string name;
    std::string sourceFile;
    ShaderLanguage language = ShaderLanguage::HLSL;

    // 各阶段入口
    struct StageEntry {
        ShaderStage stage;
        std::string entryPoint;
        std::string sourceFile; // 可选，如果不同于主文件
    };
    std::vector<StageEntry> stages;

    // 变体
    std::vector<ShaderVariant> variants;

    // 编译选项
    ShaderCompileOptions compileOptions;
};

/**
 * 着色器程序
 */
class IShaderProgram {
public:
    virtual ~IShaderProgram() = default;

    virtual const char* getName() const = 0;
    virtual const ShaderProgramDesc& getDesc() const = 0;

    // 获取特定阶段的着色器
    virtual ShaderHandle getShader(ShaderStage stage) const = 0;
    virtual ShaderHandle getShader(ShaderStage stage, const char* variant) const = 0;

    // 获取所有变体
    virtual std::vector<std::string> getVariantNames() const = 0;

    // 重新加载
    virtual bool reload() = 0;
};

/**
 * 着色器包含处理器
 */
class IShaderIncludeHandler {
public:
    virtual ~IShaderIncludeHandler() = default;

    // 查找包含文件
    virtual bool findInclude(const char* includeName,
                            const char* requestingSource,
                            std::string& outPath,
                            std::string& outContent) = 0;
};

/**
 * 着色器管理器接口
 */
class IShaderManager {
public:
    virtual ~IShaderManager() = default;

    // 初始化
    virtual bool initialize(const char* shaderRootPath) = 0;
    virtual void shutdown() = 0;

    // 编译单个着色器
    virtual ShaderCompileResult compileShader(const char* source,
                                             const ShaderCompileOptions& options) = 0;
    virtual ShaderCompileResult compileShaderFromFile(const char* filename,
                                                     const ShaderCompileOptions& options) = 0;

    // 创建着色器程序
    virtual IShaderProgram* createProgram(const ShaderProgramDesc& desc) = 0;
    virtual void destroyProgram(IShaderProgram* program) = 0;

    // 查找程序
    virtual IShaderProgram* getProgram(const char* name) = 0;

    // 包含处理器
    virtual void setIncludeHandler(IShaderIncludeHandler* handler) = 0;

    // 缓存管理
    virtual void setCacheEnabled(bool enabled) = 0;
    virtual void clearCache() = 0;
    virtual bool loadCache(const char* filename) = 0;
    virtual bool saveCache(const char* filename) = 0;

    // 热重载
    virtual void enableHotReload(bool enabled) = 0;
    virtual void checkForChanges() = 0;
    virtual void reloadAll() = 0;

    // 统计
    virtual u32 getProgramCount() const = 0;
    virtual u32 getCacheHitCount() const = 0;
    virtual u32 getCacheMissCount() const = 0;
};

/**
 * 内置着色器包含处理器
 * 从文件系统加载包含文件
 */
class FileShaderIncludeHandler : public IShaderIncludeHandler {
public:
    explicit FileShaderIncludeHandler(const std::vector<std::string>& includePaths);

    bool findInclude(const char* includeName,
                    const char* requestingSource,
                    std::string& outPath,
                    std::string& outContent) override;

    void addIncludePath(const char* path);

private:
    std::vector<std::string> includePaths;
};

} // namespace render