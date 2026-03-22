#pragma once

/**
 * RenderEngine - 抽象渲染引擎
 *
 * 这是一个基于RenderGraph架构的现代渲染引擎，支持Vulkan/D3D12等底层图形API。
 *
 * 主要特性：
 * - RenderGraph渲染图管理，自动资源生命周期和屏障管理
 * - 资源别名（Resource Aliasing）优化显存使用
 * - 多线程命令录制支持
 * - 跨平台设计，支持Windows/Linux
 *
 * 使用示例：
 * @code
 * // 1. 创建渲染系统
 * auto renderSystem = render::CreateRenderSystem(render::RenderAPI::Vulkan);
 * renderSystem->initialize(config);
 *
 * // 2. 创建渲染图
 * auto graphManager = renderSystem->getRenderGraphManager();
 * auto renderGraph = graphManager->createRenderGraph({"MainGraph"});
 *
 * // 3. 构建渲染图
 * auto builder = graphManager->beginBuild(renderGraph);
 * // ... 添加渲染通道
 * graphManager->endBuild(renderGraph);
 *
 * // 4. 执行渲染
 * graphManager->execute(renderGraph);
 * @endcode
 */

// 核心类型
#include "core/Types.h"
#include "core/Handle.h"

// 接口层
#include "interface/IRenderSystem.h"
#include "interface/IRenderDevice.h"
#include "interface/IResourceManager.h"
#include "interface/IPipelineManager.h"
#include "interface/IRenderGraph.h"
#include "interface/ICommandBuffer.h"

// 新增：高级功能模块
#include "interface/IPipelineStateCache.h"      // PSO缓存系统
#include "interface/IShaderHotReload.h"         // 着色器热重载
#include "interface/IRenderJobSystem.h"         // 多线程任务系统
#include "interface/ITextureStreaming.h"        // 纹理流送系统

// 工具类
#include "utils/Math.h"
#include "utils/Memory.h"
#include "utils/Log.h"

// 渲染系统创建
#include "render_graph/RenderSystem.h"

namespace render {

/**
 * 库版本信息
 */
struct Version {
    static constexpr u32 Major = 0;
    static constexpr u32 Minor = 1;
    static constexpr u32 Patch = 0;

    static std::string toString() {
        return std::format("{}.{}.{}", Major, Minor, Patch);
    }
};

/**
 * 初始化渲染引擎
 * 在使用任何渲染功能之前调用
 */
inline bool initializeEngine() {
    utils::Logger::get().initialize({});
    LOG_INFO("RenderEngine v{} initialized", Version::toString());
    return true;
}

/**
 * 关闭渲染引擎
 * 清理所有全局资源
 */
inline void shutdownEngine() {
    LOG_INFO("RenderEngine shutting down");
    utils::Logger::get().shutdown();
}

} // namespace render