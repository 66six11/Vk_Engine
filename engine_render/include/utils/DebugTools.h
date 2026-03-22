#pragma once

/**
 * @file DebugTools.h
 * @brief 渲染引擎调试工具总包含头文件
 *
 * 提供完整的调试、分析和性能剖析工具集：
 * - 渲染图可视化与分析 (RenderGraphDebugger)
 * - GPU资源追踪与内存分析 (GPUResourceDebugger)
 * - 性能分析器 (IRenderProfiler)
 * - 调试接口 (IRenderDebug)
 *
 * 使用示例：
 * @code
 * // 初始化调试工具
 * auto* debugger = renderSystem->getDebugInterface();
 * auto* profiler = renderSystem->getProfiler();
 *
 * // 创建渲染图调试器
 * render::RenderGraphDebugger rgDebugger(renderSystem->getRenderGraphManager());
 *
 * // 导出渲染图可视化
 * rgDebugger.exportGraph(graph, "render_graph.html", render::GraphExportFormat::HTML);
 *
 * // 性能分析
 * profiler->beginGpuTimestamp("ShadowPass");
 * // ... 渲染代码 ...
 * profiler->endGpuTimestamp("ShadowPass");
 *
 * // 导出Chrome Tracing格式
 * profiler->exportToChromeTracing("profile.json");
 * @endcode
 */

// 基础调试接口
#include "interface/IRenderSystem.h"  // IRenderDebug, IRenderProfiler

// 专用调试工具
#include "utils/RenderGraphDebugger.h"
#include "utils/GPUResourceDebugger.h"
#include "utils/Log.h"

namespace render {

/**
 * @brief 调试工具版本信息
 */
struct DebugToolsVersion {
    static constexpr u32 Major = 1;
    static constexpr u32 Minor = 0;
    static constexpr u32 Patch = 0;

    static const char* toString() {
        return "1.0.0";
    }
};

/**
 * @brief 调试工具配置
 */
struct DebugToolsConfig {
    // 通用设置
    bool enableDebugMarkers = true;
    bool enableProfiling = true;
    bool enableLeakDetection = true;

    // 渲染图调试
    bool enableRenderGraphVisualization = true;
    bool autoDumpInvalidGraphs = true;
    bool validateGraphsEveryFrame = false;

    // GPU资源调试
    bool trackResourceUsage = true;
    bool detectResourceLeaks = true;
    u32 leakDetectionThresholdFrames = 60;
    bool trackMemoryAllocations = true;

    // 性能分析
    ProfilerSamplingMode samplingMode = ProfilerSamplingMode::Frame;
    u32 samplingInterval = 1;
    u32 maxFrameHistory = 120;
    bool autoExportProfilingData = false;
    const char* autoExportPath = "profiling/";

    // 输出设置
    bool enableConsoleOutput = true;
    bool enableFileOutput = true;
    const char* outputDirectory = "debug_output/";
};

/**
 * @brief 调试工具管理器
 *
 * 集中管理所有调试工具的生命周期和配置
 */
class DebugToolsManager {
public:
    /**
     * @brief 获取单例实例
     */
    static DebugToolsManager& get();

    /**
     * @brief 初始化调试工具
     */
    void initialize(const DebugToolsConfig& config);

    /**
     * @brief 关闭调试工具
     */
    void shutdown();

    /**
     * @brief 检查是否已初始化
     */
    [[nodiscard]] bool isInitialized() const;

    /**
     * @brief 获取配置
     */
    [[nodiscard]] const DebugToolsConfig& getConfig() const;

    /**
     * @brief 更新配置
     */
    void setConfig(const DebugToolsConfig& config);

    /**
     * @brief 每帧更新（自动调用调试功能）
     */
    void update();

    /**
     * @brief 开始帧调试
     */
    void beginFrame();

    /**
     * @brief 结束帧调试
     */
    void endFrame();

    /**
     * @brief 导出所有调试数据
     */
    void exportAllData(const char* directory = nullptr);

    /**
     * @brief 生成综合调试报告
     */
    void generateReport(const char* filename);

    // 便捷访问
    [[nodiscard]] IRenderDebug* getDebugInterface() const;
    [[nodiscard]] IRenderProfiler* getProfiler() const;
    void setDebugInterface(IRenderDebug* debug);
    void setProfiler(IRenderProfiler* profiler);

private:
    DebugToolsManager() = default;
    ~DebugToolsManager() = default;

    DebugToolsManager(const DebugToolsManager&) = delete;
    DebugToolsManager& operator=(const DebugToolsManager&) = delete;

    struct Impl;
    std::unique_ptr<Impl> impl;
};

/**
 * @brief 作用域调试标记
 *
 * 自动在构造时开始调试区域，在析构时结束
 */
class ScopedDebugMarker {
public:
    ScopedDebugMarker(IRenderDebug* debug, const char* name, const Color& color)
        : debugInterface(debug) {
        if (debugInterface) {
            debugInterface->beginDebugRegion(name, color);
        }
    }

    ~ScopedDebugMarker() {
        if (debugInterface) {
            debugInterface->endDebugRegion();
        }
    }

    // 禁用拷贝和移动
    ScopedDebugMarker(const ScopedDebugMarker&) = delete;
    ScopedDebugMarker& operator=(const ScopedDebugMarker&) = delete;
    ScopedDebugMarker(ScopedDebugMarker&&) = delete;
    ScopedDebugMarker& operator=(ScopedDebugMarker&&) = delete;

private:
    IRenderDebug* debugInterface;
};

/**
 * @brief 作用域GPU时间戳
 *
 * 自动测量GPU时间
 */
class ScopedGPUTimestamp {
public:
    ScopedGPUTimestamp(IRenderProfiler* profiler, const char* name)
        : profilerInterface(profiler), timestampName(name) {
        if (profilerInterface) {
            profilerInterface->beginGpuTimestamp(timestampName);
        }
    }

    ~ScopedGPUTimestamp() {
        if (profilerInterface) {
            profilerInterface->endGpuTimestamp(timestampName);
        }
    }

    // 禁用拷贝和移动
    ScopedGPUTimestamp(const ScopedGPUTimestamp&) = delete;
    ScopedGPUTimestamp& operator=(const ScopedGPUTimestamp&) = delete;
    ScopedGPUTimestamp(ScopedGPUTimestamp&&) = delete;
    ScopedGPUTimestamp& operator=(ScopedGPUTimestamp&&) = delete;

private:
    IRenderProfiler* profilerInterface;
    const char* timestampName;
};

} // namespace render

/**
 * @defgroup DebugMacros 调试宏
 * @brief 便捷的调试宏定义
 * @{
 */

/**
 * @brief 作用域调试标记
 */
#define DEBUG_MARKER(debug, name, color) \
    render::ScopedDebugMarker _scopedMarker_##__LINE__(debug, name, color)

/**
 * @brief 作用域GPU时间戳
 */
#define GPU_TIMESTAMP(profiler, name) \
    render::ScopedGPUTimestamp _scopedTimestamp_##__LINE__(profiler, name)

/**
 * @brief 条件调试标记（仅在调试模式下）
 */
#ifdef _DEBUG
    #define DEBUG_MARKER_COND(debug, name, color) DEBUG_MARKER(debug, name, color)
    #define GPU_TIMESTAMP_COND(profiler, name) GPU_TIMESTAMP(profiler, name)
#else
    #define DEBUG_MARKER_COND(debug, name, color) ((void)0)
    #define GPU_TIMESTAMP_COND(profiler, name) ((void)0)
#endif

/**
 * @brief 快速性能测量
 */
#define PROFILE_SCOPE(profiler, name) \
    GPU_TIMESTAMP_COND(profiler, name)

/**
 * @brief 渲染通道性能测量
 */
#define PROFILE_PASS(profiler, passName) \
    GPU_TIMESTAMP_COND(profiler, passName)

/**
 * @brief 资源创建追踪
 */
#define TRACK_RESOURCE_CREATE(debugger, handle, category, size) \
    GPU_DEBUG_TRACK_CREATE(handle, category, size)

/**
 * @brief 资源销毁追踪
 */
#define TRACK_RESOURCE_DESTROY(debugger, handle) \
    GPU_DEBUG_TRACK_DESTROY(handle)

/** @} */ // DebugMacros

/**
 * @brief 调试工具版本宏
 */
#define RENDER_DEBUG_TOOLS_VERSION_MAJOR 1
#define RENDER_DEBUG_TOOLS_VERSION_MINOR 0
#define RENDER_DEBUG_TOOLS_VERSION_PATCH 0

/**
 * @brief 检查调试工具版本
 */
#define RENDER_DEBUG_TOOLS_VERSION_CHECK(major, minor, patch) \
    ((RENDER_DEBUG_TOOLS_VERSION_MAJOR > (major)) || \
     (RENDER_DEBUG_TOOLS_VERSION_MAJOR == (major) && RENDER_DEBUG_TOOLS_VERSION_MINOR > (minor)) || \
     (RENDER_DEBUG_TOOLS_VERSION_MAJOR == (major) && RENDER_DEBUG_TOOLS_VERSION_MINOR == (minor) && \
      RENDER_DEBUG_TOOLS_VERSION_PATCH >= (patch)))
