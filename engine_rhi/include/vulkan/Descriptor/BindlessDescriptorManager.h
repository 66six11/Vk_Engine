#pragma once
#ifndef ENGINE_VULKAN_BINDLESS_DESCRIPTOR_MANAGER_H
#define ENGINE_VULKAN_BINDLESS_DESCRIPTOR_MANAGER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace engine::rhi::vulkan
{
    // Bindless描述符管理器
    // 使用单个全局DescriptorSet管理所有资源（纹理、缓冲区）
    // 通过index访问，无需每帧分配/绑定Set
    class BindlessDescriptorManager
    {
        public:
            // 配置
            struct Config
            {
                uint32_t maxTextures; // 最大纹理数量
                uint32_t maxBuffers;  // 最大缓冲区数量
                uint32_t maxSamplers; // 最大采样器数量

                // 默认构造函数
                Config()
                    : maxTextures(4096)
                    , maxBuffers(1024)
                    , maxSamplers(256)
                {
                }
            };

            explicit BindlessDescriptorManager(VkDevice device);
            ~BindlessDescriptorManager();

            BindlessDescriptorManager(const BindlessDescriptorManager&)            = delete;
            BindlessDescriptorManager& operator=(const BindlessDescriptorManager&) = delete;

            BindlessDescriptorManager(BindlessDescriptorManager&&) noexcept            = default;
            BindlessDescriptorManager& operator=(BindlessDescriptorManager&&) noexcept = default;

            // 初始化Bindless Set
            bool initialize(const Config& config = {});
            void shutdown();

            // === 纹理管理 ===

            // 注册纹理，返回全局索引（用于shader访问）
            [[nodiscard]] uint32_t registerTexture(VkImageView view, VkSampler sampler);

            // 批量注册纹理
            [[nodiscard]] std::vector<uint32_t> registerTextures(
                const std::vector<std::pair<VkImageView, VkSampler>>& textures);

            // 更新已注册纹理（不分配新index）
            void updateTexture(uint32_t index, VkImageView view, VkSampler sampler);

            // 注销纹理（释放slot）
            void unregisterTexture(uint32_t index);

            // === 缓冲区管理 ===

            // 注册缓冲区，返回全局索引
            [[nodiscard]] uint32_t registerBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

            // 更新缓冲区
            void updateBuffer(uint32_t index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

            // 注销缓冲区
            void unregisterBuffer(uint32_t index);

            // === 采样器管理 ===

            // 注册采样器（可选，通常纹理自带sampler）
            [[nodiscard]] uint32_t registerSampler(VkSampler sampler);
            void                   unregisterSampler(uint32_t index);

            // === 全局Set访问 ===

            // 获取Bindless Set（每帧只需绑定一次）
            [[nodiscard]] VkDescriptorSet getGlobalSet() const { return m_globalSet; }

            // 获取SetLayout（用于Pipeline创建）
            [[nodiscard]] VkDescriptorSetLayout getLayout() const { return m_layout; }

            // 获取PipelineLayout（单Set的便捷封装）
            [[nodiscard]] VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }

            // === 统计 ===

            [[nodiscard]] uint32_t      getTextureCount() const { return m_textureCount; }
            [[nodiscard]] uint32_t      getBufferCount() const { return m_bufferCount; }
            [[nodiscard]] uint32_t      getSamplerCount() const { return m_samplerCount; }
            [[nodiscard]] const Config& getConfig() const { return m_config; }

            // === 便捷方法：常用资源索引 ===

            // 获取默认资源索引（白色纹理、默认缓冲区等）
            [[nodiscard]] uint32_t getDefaultTextureIndex() const { return m_defaultTextureIndex; }
            [[nodiscard]] uint32_t getDefaultBufferIndex() const { return m_defaultBufferIndex; }

        private:
            bool createLayout();
            bool createPool();
            bool createSet();
            bool createPipelineLayout();
            void createDefaultResources();

            // 查找空闲slot
            [[nodiscard]] uint32_t findFreeTextureSlot();
            [[nodiscard]] uint32_t findFreeBufferSlot();
            [[nodiscard]] uint32_t findFreeSamplerSlot();

            VkDevice m_device;
            Config   m_config;

            // Vulkan对象
            VkDescriptorSetLayout m_layout         = VK_NULL_HANDLE;
            VkDescriptorPool      m_pool           = VK_NULL_HANDLE;
            VkDescriptorSet       m_globalSet      = VK_NULL_HANDLE;
            VkPipelineLayout      m_pipelineLayout = VK_NULL_HANDLE;

            // 资源跟踪
            uint32_t m_textureCount = 0;
            uint32_t m_bufferCount  = 0;
            uint32_t m_samplerCount = 0;

            std::vector<bool> m_textureUsed; // slot占用标记
            std::vector<bool> m_bufferUsed;
            std::vector<bool> m_samplerUsed;

            // 默认资源索引
            uint32_t m_defaultTextureIndex = 0;
            uint32_t m_defaultBufferIndex  = 0;

            bool m_initialized = false;
    };

    // Bindless资源索引类型（类型安全）
    struct BindlessTextureIndex
    {
        uint32_t index = 0;

        explicit BindlessTextureIndex(uint32_t i) : index(i)
        {
        }

        operator uint32_t() const { return index; }
    };

    struct BindlessBufferIndex
    {
        uint32_t index = 0;

        explicit BindlessBufferIndex(uint32_t i) : index(i)
        {
        }

        operator uint32_t() const { return index; }
    };
} // namespace engine::rhi::vulkan

#endif // ENGINE_VULKAN_BINDLESS_DESCRIPTOR_MANAGER_H
