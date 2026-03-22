#pragma once

#include "interface/IRenderSystem.h"
#include "VulkanTypes.h"

namespace render::vulkan
{
    // 前向声明
    class VulkanDevice;
    class VulkanTexture;

    /**
 * Vulkan交换链实现
 */
    class VulkanSwapChain : public ISwapChain
    {
        public:
            VulkanSwapChain(VulkanDevice* device);
            ~VulkanSwapChain() override;

            bool create(const SwapChainCreateInfo& createInfo);
            void destroy();

            // ISwapChain接口实现
            TextureHandle  getCurrentBackBuffer() override;
            ResourceHandle getCurrentBackBufferResource() override;

            void present(bool vsync = true) override;
            void resize(u32 width, u32 height) override;

            u32    getWidth() const override;
            u32    getHeight() const override;
            Format getFormat() const override;
            u32    getBufferCount() const override;
            u32    getCurrentBufferIndex() const override;

            void waitForPresent() override;

            // Vulkan特定方法
            VkSwapchainKHR getVkSwapchain() const { return swapchain; }
            VkSurfaceKHR   getVkSurface() const { return surface; }
            VkSemaphore    getImageAvailableSemaphore() const;
            VkSemaphore    getRenderFinishedSemaphore() const;
            VkFence        getInFlightFence() const;

            bool acquireNextImage();
            bool presentImage();

        private:
            VulkanDevice*  device;
            VkSwapchainKHR swapchain = nullptr;
            VkSurfaceKHR   surface   = nullptr;

            // 交换链属性
            VkSurfaceFormatKHR surfaceFormat;
            VkPresentModeKHR   presentMode;
            VkExtent2D         extent;
            u32                imageCount        = 0;
            u32                currentImageIndex = 0;

            // 后备缓冲区
            std::vector<VkImage>                        images;
            std::vector<std::unique_ptr<VulkanTexture>> textures;
            std::vector<TextureHandle>                  textureHandles;

            // 同步对象
            std::vector<VkSemaphore> imageAvailableSemaphores;
            std::vector<VkSemaphore> renderFinishedSemaphores;
            std::vector<VkFence>     inFlightFences;
            std::vector<VkFence>     imagesInFlight;

            // 当前帧
            u32 currentFrame      = 0;
            u32 maxFramesInFlight = 3;

            // 窗口句柄
            void* windowHandle = nullptr;
            bool  vsync        = true;

            // 辅助函数
            bool createSurface(void* windowHandle);
            bool createSwapchain();
            bool createImageViews();
            bool createSyncObjects();

            void cleanupSwapchain();
            void recreateSwapchain();

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR   chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D         chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };
}
