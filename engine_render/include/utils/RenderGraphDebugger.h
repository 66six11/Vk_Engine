#pragma once

#include "core/Types.h"
#include "core/Handle.h"
#include "interface/IRenderGraph.h"

namespace render {

// 前向声明
class IRenderGraphManager;

/**
 * 渲染图节点可视化信息
 */
struct RenderGraphNodeInfo {
    RenderGraphPassHandle handle;
    const char* name = nullptr;
    RenderPassType type = RenderPassType::Graphics;
    u32 executionOrder = 0;
    bool culled = false;
    bool async = false;
    double gpuTimeMs = 0.0;
    std::vector<ResourceHandle> inputs;
    std::vector<ResourceHandle> outputs;
};

/**
 * 渲染图资源可视化信息
 */
struct RenderGraphResourceInfo {
    VersionedResourceHandle handle;
    const char* name = nullptr;
    ResourceType type = ResourceType::Texture2D;
    Format format = Format::RGBA8_UNORM;
    ResourceExtent extent = {1, 1, 1};
    u32 firstUsePass = 0;
    u32 lastUsePass = 0;
    u64 memorySize = 0;
    u64 memoryOffset = 0;
    bool isAliased = false;
    bool isImported = false;
    ResourceState currentState = ResourceState::Undefined;
};

/**
 * 渲染图边（依赖关系）信息
 */
struct RenderGraphEdgeInfo {
    RenderGraphPassHandle fromPass;
    RenderGraphPassHandle toPass;
    ResourceHandle resource;
    const char* dependencyType = nullptr;
};

/**
 * 渲染图统计信息
 */
struct RenderGraphStats {
    u32 totalPasses = 0;
    u32 culledPasses = 0;
    u32 asyncPasses = 0;
    u32 totalResources = 0;
    u32 transientResources = 0;
    u32 aliasedResources = 0;
    u64 totalMemory = 0;
    u64 peakMemory = 0;
    u32 barriers = 0;
    u32 mergedBarriers = 0;
    double compileTimeMs = 0.0;
    double executionTimeMs = 0.0;
};

/**
 * @brief 渲染图导出格式
 * @details 可视化导出的文件格式
 */
enum class GraphExportFormat {
    DOT,        ///< Graphviz DOT格式
    JSON,       ///< JSON格式
    Mermaid,    ///< Mermaid流程图
    HTML        ///< 交互式HTML
};

/**
 * 渲染图调试器
 * 提供渲染图可视化、分析和调试功能
 */
class RenderGraphDebugger {
public:
    explicit RenderGraphDebugger(IRenderGraphManager* graphManager);
    ~RenderGraphDebugger() = default;

    // 禁用拷贝
    RenderGraphDebugger(const RenderGraphDebugger&) = delete;
    RenderGraphDebugger& operator=(const RenderGraphDebugger&) = delete;

    // ==================== 节点查询 ====================
    [[nodiscard]] std::vector<RenderGraphNodeInfo> getNodeInfos(RenderGraphHandle graph) const;
    [[nodiscard]] RenderGraphNodeInfo getNodeInfo(RenderGraphHandle graph, RenderGraphPassHandle pass) const;
    [[nodiscard]] std::vector<RenderGraphPassHandle> getExecutionOrder(RenderGraphHandle graph) const;

    // ==================== 资源查询 ====================
    [[nodiscard]] std::vector<RenderGraphResourceInfo> getResourceInfos(RenderGraphHandle graph) const;
    [[nodiscard]] RenderGraphResourceInfo getResourceInfo(RenderGraphHandle graph, VersionedResourceHandle resource) const;
    [[nodiscard]] std::vector<RenderGraphResourceInfo> getAliasedResources(RenderGraphHandle graph) const;

    // ==================== 依赖关系 ====================
    [[nodiscard]] std::vector<RenderGraphEdgeInfo> getDependencyEdges(RenderGraphHandle graph) const;
    [[nodiscard]] std::vector<RenderGraphPassHandle> getDependencies(RenderGraphHandle graph, RenderGraphPassHandle pass) const;
    [[nodiscard]] std::vector<RenderGraphPassHandle> getDependents(RenderGraphHandle graph, RenderGraphPassHandle pass) const;

    // ==================== 统计分析 ====================
    [[nodiscard]] RenderGraphStats getStats(RenderGraphHandle graph) const;
    [[nodiscard]] double getPassGpuTime(RenderGraphHandle graph, RenderGraphPassHandle pass) const;
    [[nodiscard]] std::vector<std::pair<RenderGraphPassHandle, double>> getPassTimings(RenderGraphHandle graph) const;

    // ==================== 可视化导出 ====================
    void exportGraph(RenderGraphHandle graph, const char* filename, GraphExportFormat format) const;
    void exportToDOT(RenderGraphHandle graph, const char* filename) const;
    void exportToJSON(RenderGraphHandle graph, const char* filename) const;
    void exportToMermaid(RenderGraphHandle graph, const char* filename) const;
    void exportToHTML(RenderGraphHandle graph, const char* filename) const;

    // ==================== 内存分析 ====================
    void dumpMemoryLayout(RenderGraphHandle graph, const char* filename) const;
    void visualizeMemoryAliasing(RenderGraphHandle graph, const char* filename) const;

    // ==================== 时序分析 ====================
    void dumpExecutionTimeline(RenderGraphHandle graph, const char* filename) const;
    void dumpBarrierTimeline(RenderGraphHandle graph, const char* filename) const;

    // ==================== 验证检查 ====================
    [[nodiscard]] bool validateGraph(RenderGraphHandle graph, std::vector<std::string>* errors = nullptr) const;
    [[nodiscard]] bool detectCycles(RenderGraphHandle graph, std::vector<RenderGraphPassHandle>* cyclePath = nullptr) const;
    [[nodiscard]] bool checkResourceLeaks(RenderGraphHandle graph, std::vector<VersionedResourceHandle>* leakedResources = nullptr) const;

    // ==================== 实时调试 ====================
    void enableLivePreview(bool enable);
    void setBreakpointOnPass(const char* passName);
    void clearBreakpoint();
    void stepExecution();  // 单步执行（用于调试）

    // ==================== 性能优化建议 ====================
    [[nodiscard]] std::vector<std::string> getOptimizationSuggestions(RenderGraphHandle graph) const;
    [[nodiscard]] std::vector<std::pair<RenderGraphPassHandle, RenderGraphPassHandle>> getMergeablePasses(RenderGraphHandle graph) const;
    [[nodiscard]] std::vector<ResourceHandle> getRedundantBarriers(RenderGraphHandle graph) const;

private:
    IRenderGraphManager* graphManager;
    bool livePreviewEnabled = false;
    std::string breakpointPassName;

    // 辅助函数
    void writeDOTHeader(std::ofstream& file) const;
    void writeDOTNode(std::ofstream& file, const RenderGraphNodeInfo& node) const;
    void writeDOTEdge(std::ofstream& file, const RenderGraphEdgeInfo& edge) const;
    void writeDOTFooter(std::ofstream& file) const;
    Color getPassTypeColor(RenderPassType type) const;
};

/**
 * 便捷的渲染图调试宏
 */
#define RG_DEBUG_DUMP_GRAPH(graph, filename) \
    if (auto* debugger = render::RenderGraphDebugger::get()) { \
        debugger->exportGraph(graph, filename, render::GraphExportFormat::HTML); \
    }

#define RG_DEBUG_VALIDATE(graph) \
    if (auto* debugger = render::RenderGraphDebugger::get()) { \
        std::vector<std::string> errors; \
        if (!debugger->validateGraph(graph, &errors)) { \
            LOG_ERROR("RenderGraph validation failed:"); \
            for (const auto& error : errors) { \
                LOG_ERROR("  {}", error); \
            } \
        } \
    }

#define RG_DEBUG_PRINT_STATS(graph) \
    if (auto* debugger = render::RenderGraphDebugger::get()) { \
        auto stats = debugger->getStats(graph); \
        LOG_INFO("RenderGraph Stats: {} passes, {} resources, {}MB memory", \
            stats.totalPasses, stats.totalResources, stats.totalMemory / (1024 * 1024)); \
    }

} // namespace render
