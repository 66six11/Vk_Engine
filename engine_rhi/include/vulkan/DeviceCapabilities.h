//
// Created by C66 on 2026/3/29.
//
// 设备能力检测和特性管理
// 集中管理 Vulkan 设备功能特性，避免各 Manager 重复查询
//
#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace engine::rhi::vulkan
{
    // ==================== 设备能力结构 ====================

    struct DeviceCapabilities
    {
        // ===== Vulkan 核心版本特性 =====

        // 时间线信号量 (Vulkan 1.2)
        // 允许 GPU 执行时信号量和值等待，用于高级同步场景
        bool timelineSemaphore = false;

        // 动态渲染 (Vulkan 1.3)
        // 无需创建 RenderPass 和 Framebuffer，直接在命令缓冲中开始/结束渲染
        bool dynamicRendering = false;

        // ===== Vulkan 1.2 扩展特性 =====

        // 缓冲设备地址 (Vulkan 1.2)
        // 允许在 GPU 端通过地址直接访问缓冲区，用于光追和间接绘制
        bool bufferDeviceAddress = false;

        // 描述符索引 (Vulkan 1.2)
        // 允许在着色器中使用非统一索引访问资源数组，用于 bindless 架构
        bool descriptorIndexing = false;

        // ===== Vulkan 1.3+ 扩展特性 =====

        // 同步2 (Vulkan 1.3)
        // 提供更细粒度的同步控制，替代旧的 Pipeline Barriers
        bool synchronization2 = false;

        // ===== 可选扩展特性 =====

        // 网格着色器 (VK_EXT_mesh_shader)
        // 替代传统顶点/几何着色器，用于微网格和 Nanite 类技术
        bool meshShader = false;

        // 光线追踪管线 (VK_KHR_ray_tracing_pipeline)
        bool rayTracingPipeline = false;

        // 加速结构 (VK_KHR_acceleration_structure)
        bool accelerationStructure = false;

        // 光线查询 (VK_KHR_ray_query)
        // 在着色器中直接查询加速结构，无需完整光追管线
        bool rayQuery = false;

        // ===== 工具方法 =====

        // 检查是否支持任何光追功能
        bool SupportsRayTracing() const
        {
            return rayTracingPipeline || rayQuery;
        }

        // 检查是否支持 bindless 资源
        bool SupportsBindless() const
        {
            return descriptorIndexing && bufferDeviceAddress;
        }
    };

    // ==================== 特性启用请求结构 ====================

    // 用于创建设备时请求启用哪些特性
    struct FeatureRequirements
    {
        // 必需的最低 Vulkan API 版本
        uint32_t minApiVersion = VK_API_VERSION_1_2;

        // 请求启用的特性
        bool requireTimelineSemaphore      = false;
        bool requireDynamicRendering       = false;
        bool requireBufferDeviceAddress    = false;
        bool requireDescriptorIndexing     = false;
        bool requireSynchronization2       = false;
        bool requireMeshShader             = false;
        bool requireRayTracing             = false;

        // 检查请求的特性和设备能力是否匹配
        bool IsCompatible(const DeviceCapabilities& caps) const
        {
            if (requireTimelineSemaphore && !caps.timelineSemaphore)
                return false;
            if (requireDynamicRendering && !caps.dynamicRendering)
                return false;
            if (requireBufferDeviceAddress && !caps.bufferDeviceAddress)
                return false;
            if (requireDescriptorIndexing && !caps.descriptorIndexing)
                return false;
            if (requireSynchronization2 && !caps.synchronization2)
                return false;
            if (requireMeshShader && !caps.meshShader)
                return false;
            if (requireRayTracing && !caps.SupportsRayTracing())
                return false;
            return true;
        }
    };

} // namespace engine::rhi::vulkan
