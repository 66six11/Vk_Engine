#pragma once

#include "interface/IRenderSystem.h"

namespace render {

// 前向声明
class IRenderDevice;
class IResourceManager;
class IPipelineManager;
class IRenderGraphManager;
class IShaderManager;
class IRenderDebug;
class IRenderProfiler;
class ISwapChain;
class ICommandQueue;

namespace render_graph {

// 前向声明
class RenderGraph;

/**
 * RenderGraph渲染系统实现
 * 整合所有子系统，提供统一的渲染接口
 */
class RenderSystem : public IRenderSystem {
public:
    RenderSystem();
    ~RenderSystem() override;

    // IRenderSystem接口实现
    bool initialize(const RenderSystemConfig& config) override;
    void shutdown() override;

    void beginFrame(const FrameInfo& frameInfo) override;
    void endFrame() override;
    void present(ISwapChain* swapChain) override;

    IRenderGraphManager* getRenderGraphManager() override;
    IResourceManager* getResourceManager() override;
    IPipelineManager* getPipelineManager() override;
    IShaderManager* getShaderManager() override;
    IRenderDevice* getRenderDevice() override;

    RenderAPI getRenderAPI() const override;
    const DeviceInfo& getDeviceInfo() const override;

    IRenderDebug* getDebugInterface() override;
    IRenderProfiler* getProfiler() override;

    ICommandQueue* getGraphicsQueue(u32 index) override;
    ICommandQueue* getComputeQueue(u32 index) override;
    ICommandQueue* getTransferQueue(u32 index) override;

    ISwapChain* createSwapChain(const SwapChainCreateInfo& createInfo) override;
    void destroySwapChain(ISwapChain* swapChain) override;

    void waitIdle() override;
    void flush() override;

    MemoryStatistics getMemoryStatistics() const override;

    // 高级功能
    void setVSyncEnabled(bool enabled);
    bool isVSyncEnabled() const;

    void setHDRMode(bool enabled);
    bool isHDRMode() const;

    // 截图
    bool captureScreenshot(const char* filename);

    // 视频录制（可选）
    bool beginVideoCapture(const char* filename, u32 fps);
    void endVideoCapture();
    bool isVideoCapturing() const;

private:
    // 子系统
    std::unique_ptr<IRenderDevice> device;
    std::unique_ptr<IResourceManager> resourceManager;
    std::unique_ptr<IPipelineManager> pipelineManager;
    std::unique_ptr<IRenderGraphManager> renderGraphManager;
    std::unique_ptr<IShaderManager> shaderManager;
    std::unique_ptr<IRenderDebug> debugInterface;
    std::unique_ptr<IRenderProfiler> profiler;

    // 配置
    RenderSystemConfig config;
    RenderAPI api;

    // 状态
    bool initialized = false;
    bool vsyncEnabled = true;
    bool hdrMode = false;

    // 帧管理
    FrameInfo currentFrameInfo;
    u64 frameNumber = 0;

    // 主交换链
    ISwapChain* mainSwapChain = nullptr;

    // 视频录制
    bool videoCapturing = false;

    // 初始化步骤
    bool createDevice();
    bool createResourceManager();
    bool createPipelineManager();
    bool createRenderGraphManager();
    bool createShaderManager();
    bool createDebugInterface();
    bool createProfiler();
    bool createCommandQueues();  // 创建命令队列

    // 队列管理
    bool findQueueFamilyIndices();  // 查找队列族索引
    bool createQueue(QueueType type, u32 familyIndex, u32 count);

    // 清理
    void destroySubsystems();
    void destroyCommandQueues();
};

/**
 * 渲染系统创建函数
 */
std::unique_ptr<IRenderSystem> CreateRenderSystem(RenderAPI api);
std::unique_ptr<IRenderSystem> CreateRenderSystem();

} // namespace render_graph

// 导出到render命名空间
using render_graph::CreateRenderSystem;

} // namespace render