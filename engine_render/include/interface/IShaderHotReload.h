#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include <functional>

namespace render {

// 前向声明
class IShaderManager;
class IPipelineManager;

/**
 * @brief 着色器文件变更类型
 * @details 文件系统监视检测到的变更类型
 */
enum class ShaderFileChangeType : u8 {
    Created,    ///< 文件创建
    Modified,   ///< 文件修改（热重载触发）
    Deleted,    ///< 文件删除
    Renamed     ///< 文件重命名
};

/**
 * @brief 着色器编译状态
 * @details 热重载编译的结果状态
 */
enum class ShaderCompileStatus : u8 {
    Success,            ///< 编译成功
    SyntaxError,        ///< 语法错误
    LinkError,          ///< 链接错误
    IncludeError,       ///< 包含文件错误
    FileNotFound,       ///< 文件未找到
    UnknownError        ///< 未知错误
};

/**
 * 着色器错误信息
 */
struct ShaderErrorInfo {
    const char* filePath = nullptr;
    u32 lineNumber = 0;
    u32 columnNumber = 0;
    const char* errorMessage = nullptr;
    ShaderCompileStatus status = ShaderCompileStatus::UnknownError;
};

/**
 * 着色器变体定义
 */
struct ShaderVariantDef {
    const char* name = nullptr;
    const char* value = nullptr;
};

/**
 * 着色器变体描述
 */
struct ShaderVariantDesc {
    ShaderHandle baseShader;
    const char* variantName = nullptr;
    std::vector<ShaderVariantDef> defines;
    u64 variantKey = 0;  // 自动计算
};

/**
 * 着色器变体信息
 */
struct ShaderVariantInfo {
    ShaderHandle handle;
    ShaderHandle baseShader;
    const char* variantName = nullptr;
    std::vector<ShaderVariantDef> defines;
    u64 variantKey = 0;
    u32 refCount = 0;
};

/**
 * 热重载配置
 */
struct ShaderHotReloadConfig {
    // 文件监控
    bool enableFileWatcher = true;
    u32 fileWatcherIntervalMs = 500;  // 轮询间隔
    
    // 重载行为
    bool autoReload = true;           // 自动重载
    bool compileInBackground = true;  // 后台编译
    bool preserveRenderState = true;  // 保留渲染状态
    
    // 错误处理
    bool showErrorPopup = true;       // 显示错误弹窗
    u32 maxErrorHistory = 100;        // 最大错误历史
    
    // 变体管理
    bool enableVariantManagement = true;
    u32 maxVariantsPerShader = 256;   // 每着色器最大变体数
    
    // 调试
    bool logReloadEvents = true;
    bool keepBackupShaders = false;   // 保留备份
};

/**
 * 着色器热重载回调
 */
using ShaderReloadCallback = std::function<void(
    ShaderHandle shader,              // 被重载的着色器
    bool success,                     // 是否成功
    const std::vector<ShaderErrorInfo>& errors,  // 错误信息
    void* userData)>;

/**
 * 变体收集回调（用于PSO预热）
 */
using VariantCollectCallback = std::function<void(
    const ShaderVariantDesc& variantDesc,
    void* userData)>;

/**
 * 着色器热重载接口
 * 
 * 提供着色器热重载、变体管理、错误处理等功能
 * 
 * 使用示例：
 * @code
 * // 1. 创建热重载系统
 * auto hotReload = renderSystem->createShaderHotReload(config);
 * 
 * // 2. 注册着色器（启用热重载）
 * ShaderHandle shader = shaderMgr->createShader({...});
 * hotReload->registerShader(shader, "shaders/my_shader.hlsl");
 * 
 * // 3. 注册热重载回调
 * hotReload->registerReloadCallback(shader, [](ShaderHandle handle, bool success, 
 *                                               const auto& errors, void* data) {
 *     if (success) {
 *         LOG_INFO("Shader reloaded successfully!");
 *     } else {
 *         for (const auto& error : errors) {
 *             LOG_ERROR("Shader error: {}:{}", error.filePath, error.lineNumber);
 *         }
 *     }
 * });
 * 
 * // 4. 启用文件监控
 * hotReload->startFileWatcher();
 * 
 * // 5. 每帧更新（处理文件变更）
 * hotReload->update();
 * @endcode
 */
class IShaderHotReload {
public:
    virtual ~IShaderHotReload() = default;

    // ==================== 初始化与配置 ====================
    
    /**
     * @brief 初始化热重载系统
     * @param shaderManager 着色器管理器
     * @param pipelineManager 管线管理器（用于重建PSO）
     * @param config 配置
     */
    virtual bool initialize(IShaderManager* shaderManager,
                           IPipelineManager* pipelineManager,
                           const ShaderHotReloadConfig& config) = 0;
    
    /**
     * @brief 关闭热重载系统
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 更新配置
     */
    virtual void setConfig(const ShaderHotReloadConfig& config) = 0;
    
    /**
     * @brief 获取当前配置
     */
    [[nodiscard]] virtual const ShaderHotReloadConfig& getConfig() const = 0;

    // ==================== 着色器注册 ====================
    
    /**
     * @brief 注册着色器到热重载系统
     * @param shader 着色器句柄
     * @param filePath 源文件路径
     * @param entryPoint 入口函数名（可空）
     * @param shaderModel 着色器模型（可空）
     * @return 是否成功
     */
    virtual bool registerShader(ShaderHandle shader,
                               const char* filePath,
                               const char* entryPoint = nullptr,
                               const char* shaderModel = nullptr) = 0;
    
    /**
     * @brief 注销着色器
     */
    virtual void unregisterShader(ShaderHandle shader) = 0;
    
    /**
     * @brief 获取着色器对应的文件路径
     */
    [[nodiscard]] virtual const char* getShaderFilePath(ShaderHandle shader) const = 0;
    
    /**
     * @brief 获取文件路径对应的所有着色器
     */
    [[nodiscard]] virtual std::vector<ShaderHandle> getShadersByFilePath(
        const char* filePath) const = 0;
    
    /**
     * @brief 检查着色器是否已注册
     */
    [[nodiscard]] virtual bool isShaderRegistered(ShaderHandle shader) const = 0;

    // ==================== 热重载回调 ====================
    
    /**
     * @brief 注册重载回调
     * @param shader 着色器句柄（InvalidHandle表示监听所有）
     * @param callback 回调函数
     * @param userData 用户数据
     * @return 回调ID
     */
    [[nodiscard]] virtual u32 registerReloadCallback(
        ShaderHandle shader,
        ShaderReloadCallback callback,
        void* userData = nullptr) = 0;
    
    /**
     * @brief 注销重载回调
     */
    virtual void unregisterReloadCallback(u32 callbackId) = 0;
    
    /**
     * @brief 注册编译前回调
     * @param callback 回调函数
     * @note 返回false可取消编译
     */
    virtual void registerPreCompileCallback(
        std::function<bool(ShaderHandle)> callback) = 0;
    
    /**
     * @brief 注册编译后回调
     */
    virtual void registerPostCompileCallback(
        std::function<void(ShaderHandle, bool)> callback) = 0;

    // ==================== 文件监控 ====================
    
    /**
     * @brief 启动文件监控
     */
    virtual void startFileWatcher() = 0;
    
    /**
     * @brief 停止文件监控
     */
    virtual void stopFileWatcher() = 0;
    
    /**
     * @brief 检查文件监控是否运行
     */
    [[nodiscard]] virtual bool isFileWatcherRunning() const = 0;
    
    /**
     * @brief 手动触发检查（不使用文件监控时）
     * @return 变更的文件数量
     */
    virtual u32 manualCheck() = 0;
    
    /**
     * @brief 处理文件变更事件
     * @param filePath 文件路径
     * @param changeType 变更类型
     */
    virtual void onFileChanged(const char* filePath, 
                              ShaderFileChangeType changeType) = 0;

    // ==================== 手动重载 ====================
    
    /**
     * @brief 手动重载着色器
     * @param shader 着色器句柄
     * @param force 是否强制重载（即使文件未变更）
     * @return 是否成功
     */
    virtual bool reloadShader(ShaderHandle shader, bool force = false) = 0;
    
    /**
     * @brief 批量重载
     * @param shaders 着色器列表
     * @param progressCallback 进度回调
     * @return 成功重载的数量
     */
    virtual u32 reloadShaders(const std::vector<ShaderHandle>& shaders,
                             std::function<void(u32, u32)> progressCallback = nullptr) = 0;
    
    /**
     * @brief 重载所有着色器
     */
    virtual u32 reloadAllShaders() = 0;

    // ==================== 着色器变体管理 ====================
    
    /**
     * @brief 创建着色器变体
     * @param baseShader 基础着色器
     * @param variantDesc 变体描述
     * @return 变体着色器句柄
     * @note 变体会继承基础着色器的文件监控
     */
    [[nodiscard]] virtual ShaderHandle createVariant(
        ShaderHandle baseShader,
        const ShaderVariantDesc& variantDesc) = 0;
    
    /**
     * @brief 创建多个变体
     * @param baseShader 基础着色器
     * @param definesList 宏定义组合列表
     * @return 变体着色器列表
     */
    [[nodiscard]] virtual std::vector<ShaderHandle> createVariants(
        ShaderHandle baseShader,
        const std::vector<std::vector<ShaderVariantDef>>& definesList) = 0;
    
    /**
     * @brief 获取变体信息
     */
    [[nodiscard]] virtual ShaderVariantInfo getVariantInfo(ShaderHandle variant) const = 0;
    
    /**
     * @brief 获取基础着色器的所有变体
     */
    [[nodiscard]] virtual std::vector<ShaderHandle> getVariants(ShaderHandle baseShader) const = 0;
    
    /**
     * @brief 收集材质使用的所有变体（用于预热）
     * @param material 材质
     * @param callback 变体收集回调
     * @note 分析材质使用的所有关键字组合
     */
    virtual void collectMaterialVariants( IMaterial* material,
                                         VariantCollectCallback callback) = 0;
    
    /**
     * @brief 预热着色器变体
     * @param baseShader 基础着色器
     * @param variants 变体定义列表
     * @return 成功预热的数量
     */
    virtual u32 warmupVariants(ShaderHandle baseShader,
                              const std::vector<ShaderVariantDesc>& variants) = 0;
    
    /**
     * @brief 清理未使用的变体
     * @param minFramesUnused 最小未使用帧数
     * @return 清理的数量
     */
    virtual u32 purgeUnusedVariants(u32 minFramesUnused = 300) = 0;
    
    /**
     * @brief 获取变体统计
     */
    [[nodiscard]] virtual std::pair<u32, u32> getVariantStats(ShaderHandle baseShader) const = 0;

    // ==================== 错误处理 ====================
    
    /**
     * @brief 获取上次编译错误
     */
    [[nodiscard]] virtual std::vector<ShaderErrorInfo> getLastErrors(ShaderHandle shader) const = 0;
    
    /**
     * @brief 获取所有错误历史
     */
    [[nodiscard]] virtual std::vector<ShaderErrorInfo> getErrorHistory() const = 0;
    
    /**
     * @brief 清除错误历史
     */
    virtual void clearErrorHistory() = 0;
    
    /**
     * @brief 导出错误报告
     */
    virtual void exportErrorReport(const char* filename) const = 0;
    
    /**
     * @brief 获取着色器源码位置（调试用）
     */
    [[nodiscard]] virtual std::string getShaderSourceWithLineNumbers(
        ShaderHandle shader) const = 0;

    // ==================== 渲染状态保留 ====================
    
    /**
     * @brief 开始重载会话
     * @note 自动保存所有渲染状态，重载后恢复
     */
    virtual void beginReloadSession() = 0;
    
    /**
     * @brief 结束重载会话
     */
    virtual void endReloadSession() = 0;
    
    /**
     * @brief 是否在重载会话中
     */
    [[nodiscard]] virtual bool isInReloadSession() const = 0;

    // ==================== 每帧更新 ====================
    
    /**
     * @brief 更新（处理文件变更和异步编译）
     */
    virtual void update() = 0;
    
    /**
     * @brief 等待所有异步编译完成
     */
    virtual void waitForAsyncCompiles() = 0;

    // ==================== 调试与统计 ====================
    
    /**
     * @brief 获取监控的文件数量
     */
    [[nodiscard]] virtual u32 getWatchedFileCount() const = 0;
    
    /**
     * @brief 获取注册的着色器数量
     */
    [[nodiscard]] virtual u32 getRegisteredShaderCount() const = 0;
    
    /**
     * @brief 导出监控文件列表
     */
    virtual void dumpWatchedFiles(const char* filename) const = 0;
    
    /**
     * @brief 打印统计信息
     */
    virtual void printStats() const = 0;
};

/**
 * @brief 着色器热重载工厂
 */
class ShaderHotReloadFactory {
public:
    static std::unique_ptr<IShaderHotReload> create();
    static std::unique_ptr<IShaderHotReload> create(const ShaderHotReloadConfig& config);
};

// ==================== 便捷宏 ====================

/**
 * @brief 注册着色器热重载
 */
#define REGISTER_SHADER_HOTRELOAD(hotReload, shader, filePath) \
    (hotReload)->registerShader((shader), (filePath))

/**
 * @brief 快速重载
 */
#define RELOAD_SHADER(hotReload, shader) \
    do { \
        LOG_INFO("Reloading shader: {}", #shader); \
        bool success = (hotReload)->reloadShader((shader), true); \
        if (success) { \
            LOG_INFO("Shader reloaded successfully"); \
        } else { \
            LOG_ERROR("Failed to reload shader"); \
        } \
    } while(0)

/**
 * @brief 创建变体（单一定义）
 */
#define CREATE_SHADER_VARIANT(hotReload, baseShader, variantName, defineName, defineValue) \
    (hotReload)->createVariant((baseShader), \
        render::ShaderVariantDesc{.variantName = (variantName), \
            .defines = {{(defineName), (defineValue)}}})

/**
 * @brief 开始热重载会话（RAII）
 */
class ShaderReloadSession {
public:
    explicit ShaderReloadSession(IShaderHotReload* hotReload) 
        : hotReload_(hotReload) {
        if (hotReload_) {
            hotReload_->beginReloadSession();
        }
    }
    
    ~ShaderReloadSession() {
        if (hotReload_) {
            hotReload_->endReloadSession();
        }
    }
    
    // 禁用拷贝和移动
    ShaderReloadSession(const ShaderReloadSession&) = delete;
    ShaderReloadSession& operator=(const ShaderReloadSession&) = delete;
    ShaderReloadSession(ShaderReloadSession&&) = delete;
    ShaderReloadSession& operator=(ShaderReloadSession&&) = delete;
    
private:
    IShaderHotReload* hotReload_;
};

/**
 * @brief 作用域热重载会话
 */
#define SHADER_RELOAD_SESSION(hotReload) \
    render::ShaderReloadSession _shaderSession_##__LINE__(hotReload)

} // namespace render
