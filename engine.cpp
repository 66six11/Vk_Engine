#include "engine.h"
#include <iostream>
#include "RHIFwd.h"
#include "vulkan/DeviceManager.h"
#include "vulkan/SwapChainManager.h"

// GLFW
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// 创建 GLFW 窗口
GLFWwindow* CreateGLFWWindow(uint32_t width, uint32_t height)
{
    if (!glfwInit())
    {
        return nullptr;
    }

    // 设置 GLFW 不创建 OpenGL 上下文（纯 Vulkan）
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(width),
        static_cast<int>(height),
        "Engine SwapChain Test",
        nullptr,
        nullptr
    );

    return window;
}

void engine_init()
{
    std::cout << "Engine initializing..." << std::endl;

    // 创建并初始化 DeviceManager
    engine::rhi::vulkan::DeviceManager deviceManager;
    try
    {
        deviceManager.Initialize();
        std::cout << "DeviceManager initialized successfully!" << std::endl;

        // 输出设备信息
        std::cout << "Physical Device: " << deviceManager.GetPhysicalDevice() << std::endl;
        std::cout << "Logical Device: " << deviceManager.GetLogicalDevice() << std::endl;
        std::cout << "Instance: " << deviceManager.GetInstance() << std::endl;

        // 输出队列族信息
        const auto& indices = deviceManager.GetQueueFamilyIndices();
        std::cout << "Graphics Queue Family: " << indices.graphicsFamily << std::endl;
        std::cout << "Compute Queue Family: " << indices.computeFamily << std::endl;
        std::cout << "Transfer Queue Family: " << indices.transferFamily << std::endl;
        std::cout << "Is Unified Queue: " << (indices.IsUnifiedQueue() ? "Yes" : "No") << std::endl;

        // ===== 测试 SwapChain =====
        std::cout << "\n=== Testing SwapChain ===" << std::endl;

        // 创建 GLFW 窗口
        GLFWwindow* window = CreateGLFWWindow(1280, 720);
        if (!window)
        {
            throw std::runtime_error("Failed to create GLFW window");
        }
        std::cout << "GLFW window created" << std::endl;

        // 获取原生窗口句柄 (Windows)
        HWND hwnd = glfwGetWin32Window(window);
        if (!hwnd)
        {
            glfwDestroyWindow(window);
            glfwTerminate();
            throw std::runtime_error("Failed to get Win32 window handle");
        }

        // 创建 SwapChainManager
        engine::rhi::vulkan::SwapChainManager swapChainManager(deviceManager);

        // 配置 SwapChain
        engine::rhi::vulkan::SwapChainConfig config;
        config.width = 1280;
        config.height = 720;
        config.imageCount = 3;
        config.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        config.preferredFormat = VK_FORMAT_B8G8R8A8_UNORM;

        // 初始化 SwapChain
        swapChainManager.Initialize(config, hwnd);
        std::cout << "SwapChain initialized successfully!" << std::endl;

        // 输出 SwapChain 信息
        std::cout << "SwapChain Format: " << swapChainManager.GetFormat() << std::endl;
        std::cout << "SwapChain Extent: " << swapChainManager.GetExtent().width << "x" << swapChainManager.GetExtent().height << std::endl;
        std::cout << "SwapChain Image Count: " << swapChainManager.GetImageCount() << std::endl;
        std::cout << "SwapChain Images: " << swapChainManager.GetImages().size() << std::endl;

        // 清理 SwapChain
        swapChainManager.Shutdown();
        std::cout << "SwapChain shutdown." << std::endl;

        // 销毁 GLFW 窗口
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "GLFW window destroyed." << std::endl;

        // 清理 DeviceManager
        deviceManager.Shutdown();
        std::cout << "DeviceManager shutdown." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize: " << e.what() << std::endl;
        throw;
    }
}

int main()
{
    std::cout << "Engine starting..." << std::endl;

    try
    {
        engine_init();
        std::cout << "Engine initialized successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Engine initialization failed: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
