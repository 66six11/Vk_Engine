//
// Created by C66 on 2026/3/23.
//
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace engine::rhi::vulkan
{
    class DeviceManager;

    struct SwapChainConfig
    {
        uint32_t         width           = 1280;
        uint32_t         height          = 720;
        uint32_t         imageCount      = 3;                        // 三缓冲
        VkPresentModeKHR presentMode     = VK_PRESENT_MODE_FIFO_KHR; // VSync
        VkFormat         preferredFormat = VK_FORMAT_B8G8R8A8_UNORM;
    };

    struct SwapChainImage
    {
        VkImage     image = VK_NULL_HANDLE; // SwapChain 拥有，不需要销毁
        VkImageView view  = VK_NULL_HANDLE; // 需要手动创建/销毁
        uint32_t    index = 0;
    };

    class SwapChainManager
    {
        private:
            DeviceManager& deviceManager_; //设备管理器

            VkSwapchainKHR swapChain = VK_NULL_HANDLE;
            VkSurfaceKHR   surface   = VK_NULL_HANDLE;

            VkFormat         format_     = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR  colorSpace_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkExtent2D       extent_{}; // 交换链图像的分辨率
            VkPresentModeKHR presentMode_         = VK_PRESENT_MODE_FIFO_KHR;
            uint32_t         requestedImageCount_ = 3; // 三缓冲图像

            // Image 数据
            std::vector<SwapChainImage> images_;
            uint32_t                    currentImageIndex_ = 0;

            // 用于 Recreate 的旧 SwapChain
            VkSwapchainKHR oldSwapChain_ = VK_NULL_HANDLE;


            void CreateSurface(void* nativeWindowHandle);
            void CreateSwapChain(const SwapChainConfig& config);

        public:
            explicit SwapChainManager(DeviceManager& deviceManager);
            ~SwapChainManager();
            // 禁止拷贝
            SwapChainManager(const SwapChainManager&)            = delete;
            SwapChainManager& operator=(const SwapChainManager&) = delete;

            //禁止移动
            SwapChainManager(SwapChainManager&& other)            = delete;
            SwapChainManager& operator=(SwapChainManager&& other) = delete;

            // 初始化/销毁
            void Initialize(const SwapChainConfig& config, void* nativeWindowHandle);
            void Shutdown();

            //重建
            void Recreate();

            //getter
            VkSwapchainKHR               GetSwapChain() { return swapChain; }
            VkFormat                     GetFormat() { return format_; }
            VkColorSpaceKHR              GetColorSpace() { return colorSpace_; }
            VkExtent2D                   GetExtent() { return extent_; }
            VkPresentModeKHR             GetPresentMode() { return presentMode_; }
            uint32_t                     GetImageCount() { return requestedImageCount_; }
            std::vector<SwapChainImage>& GetImages() { return images_; }
            uint32_t                     GetCurrentImageIndex() { return currentImageIndex_; }
            VkImage                      GetCurrentImage() { return images_[currentImageIndex_].image; }
            VkImageView                  GetCurrentImageView() { return images_[currentImageIndex_].view; }
            VkImageView                  GetImageView(uint32_t index) { return images_[index].view; }
            //Extent setter
            void SetExtent(const VkExtent2D& extent){ extent_ = extent;};
            
    };
}
