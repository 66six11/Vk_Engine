#include <algorithm>
#include <iostream>

#include "vulkan/SwapChainManager.h"
#include "vulkan/DeviceManager.h"
#include "vulkan/VulkanUtils.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

namespace engine::rhi::vulkan
{
    void SwapChainManager::CreateSurface(void* nativeWindowHandle)
    {
        if (!nativeWindowHandle)
        {
            throw std::runtime_error("Native window handle is null");
        }

        VkInstance instance = deviceManager_.GetInstance();

        #ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd                        = static_cast<HWND>(nativeWindowHandle);
        createInfo.hinstance                   = GetModuleHandle(nullptr);

        VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
        #else
        // Linux 平台待实现
        #error "Platform not supported"
        #endif
    }

    void SwapChainManager::CreateSwapChain(const SwapChainConfig& config)
    {
        VkPhysicalDevice physicalDevice = deviceManager_.GetPhysicalDevice();
        VkDevice         device         = deviceManager_.GetLogicalDevice();

        // 1. 查询 Surface 能力
        VkSurfaceCapabilitiesKHR capabilities;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities));

        // 2. 查询支持的格式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        // 3. 查询支持的呈现模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        // 4. 选择 Surface 格式
        VkSurfaceFormatKHR selectedFormat = formats[0];
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == config.preferredFormat &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                selectedFormat = availableFormat;
                break;
            }
        }
        format_     = selectedFormat.format;
        colorSpace_ = selectedFormat.colorSpace;

        // 5. 选择呈现模式
        presentMode_ = VK_PRESENT_MODE_FIFO_KHR; // 默认 VSync
        for (const auto& availablePresentMode : presentModes)
        {
            if (availablePresentMode == config.presentMode)
            {
                presentMode_ = availablePresentMode;
                break;
            }
        }

        // 6. 确定交换范围
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            extent_ = capabilities.currentExtent;
        }
        else
        {
            extent_.width  = std::clamp(config.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent_.height = std::clamp(config.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        // 7. 确定图像数量
        requestedImageCount_ = config.imageCount;
        if (requestedImageCount_ < capabilities.minImageCount)
        {
            requestedImageCount_ = capabilities.minImageCount;
        }
        if (capabilities.maxImageCount > 0 && requestedImageCount_ > capabilities.maxImageCount)
        {
            requestedImageCount_ = capabilities.maxImageCount;
        }

        // 8. 创建交换链
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
        swapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface                  = surface;
        swapchainCreateInfo.minImageCount            = requestedImageCount_;
        swapchainCreateInfo.imageFormat              = format_;
        swapchainCreateInfo.imageColorSpace          = colorSpace_;
        swapchainCreateInfo.imageExtent              = extent_;
        swapchainCreateInfo.imageArrayLayers         = 1;
        swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // 9. 确定队列族索引
        uint32_t queueFamilyIndices[]             = {deviceManager_.GetGraphicsQueueFamily()};
        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            deviceManager_.GetPhysicalDevice(),
            queueFamilyIndices[0],  // 假设用这个
            surface,
            &presentSupport
        );
       
        if (!presentSupport) {
            throw std::runtime_error("Graphics queue family does not support presentation");
        }

        swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 1;
        swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;

        swapchainCreateInfo.preTransform   = capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode    = presentMode_;
        swapchainCreateInfo.clipped        = VK_TRUE;
        swapchainCreateInfo.oldSwapchain   = oldSwapChain_;

        VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapChain));

        // 10. 如果存在旧交换链，销毁它
        if (oldSwapChain_ != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, oldSwapChain_, nullptr);
            oldSwapChain_ = VK_NULL_HANDLE;
        }

        // 11. 获取交换链图像
        uint32_t imageCount;
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr));
        std::vector<VkImage> swapChainImages(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data()));

        // 12. 创建 ImageView
        images_.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            images_[i].image = swapChainImages[i];
            images_[i].index = i;

            VkImageViewCreateInfo viewCreateInfo           = {};
            viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.image                           = swapChainImages[i];
            viewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            viewCreateInfo.format                          = format_;
            viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            viewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            viewCreateInfo.subresourceRange.baseMipLevel   = 0;
            viewCreateInfo.subresourceRange.levelCount     = 1;
            viewCreateInfo.subresourceRange.baseArrayLayer = 0;
            viewCreateInfo.subresourceRange.layerCount     = 1;

            VK_CHECK(vkCreateImageView(device, &viewCreateInfo, nullptr, &images_[i].view));
        }
    }


    void SwapChainManager::Initialize(const SwapChainConfig& config, void* nativeWindowHandle)
    {
        CreateSurface(nativeWindowHandle);
        CreateSwapChain(config);
    }

    void SwapChainManager::Shutdown()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 销毁 ImageView
        for (auto& image : images_)
        {
            if (image.view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device, image.view, nullptr);
                image.view = VK_NULL_HANDLE;
            }
        }
        images_.clear();

        // 销毁 SwapChain
        if (swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, swapChain, nullptr);
            swapChain = VK_NULL_HANDLE;
        }

        // 销毁旧 SwapChain（如果存在）
        if (oldSwapChain_ != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, oldSwapChain_, nullptr);
            oldSwapChain_ = VK_NULL_HANDLE;
        }

        // 销毁 Surface
        if (surface != VK_NULL_HANDLE)
        {
            VkInstance instance = deviceManager_.GetInstance();
            vkDestroySurfaceKHR(instance, surface, nullptr);
            surface = VK_NULL_HANDLE;
        }
    }
    

    void SwapChainManager::Recreate()
    {
        VkDevice device = deviceManager_.GetLogicalDevice();

        // 等待 GPU 完成所有操作
        vkDeviceWaitIdle(device);

        // 销毁旧的 ImageViews
        for (auto& image : images_)
        {
            if (image.view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device, image.view, nullptr);
                image.view = VK_NULL_HANDLE;
            }
        }
        images_.clear();

        // 将当前 SwapChain 标记为旧 SwapChain
        // CreateSwapChain 会处理它的销毁
        if (swapChain != VK_NULL_HANDLE)
        {
            oldSwapChain_ = swapChain;
            swapChain     = VK_NULL_HANDLE;
        }

        // 重新创建 SwapChain（保留当前配置）
        SwapChainConfig config;
        config.width           = extent_.width;
        config.height          = extent_.height;
        config.imageCount      = requestedImageCount_;
        config.presentMode     = presentMode_;
        config.preferredFormat = format_;

        CreateSwapChain(config);
    }

    SwapChainManager::SwapChainManager(DeviceManager& deviceManager) : deviceManager_(deviceManager)
    {
        // 暂不实现
    }

    SwapChainManager::~SwapChainManager()
    {
        // 确保资源被清理
        // 注意：如果 DeviceManager 在 SwapChainManager 之前被销毁会出错
        // 通常应在 Shutdown() 中显式调用
    }
}
