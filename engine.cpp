#include "engine.h"
#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include <cmath>

// RHI Headers
#include "vulkan/DeviceManager.h"
#include "vulkan/SwapChainManager.h"
#include "vulkan/Command/CommandPool.h"
#include "vulkan/Command/CommandBuffer.h"
#include "vulkan/Command/CommandQueue.h"
#include "vulkan/Resources/BufferManager.h"
#include "vulkan/Resources/TextureManager.h"
#include "vulkan/Resources/SamplerManager.h"
#include "vulkan/Resources/ShaderManager.h"
#include "vulkan/Descriptor/BindlessDescriptorManager.h"
#include "vulkan/RenderPipeline/PipelineLayoutManager.h"
#include "vulkan/RenderPipeline/GraphicsPipelineManager.h"
#include "vulkan/Synchronization/SynchronizationManager.h"
#include "vulkan/VulkanUtils.h"

// GLFW
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace engine {

// 顶点结构
struct Vertex {
    float position[3];
    float color[3];
};

// Uniform数据
struct UniformBufferObject {
    float mvp[16];  // Model-View-Projection矩阵
};

// 全局RHI对象
std::unique_ptr<rhi::vulkan::DeviceManager> g_deviceManager;
std::unique_ptr<rhi::vulkan::SwapChainManager> g_swapChainManager;
std::unique_ptr<rhi::vulkan::BufferManager> g_bufferManager;
std::unique_ptr<rhi::vulkan::TextureManager> g_textureManager;
std::unique_ptr<rhi::vulkan::SamplerManager> g_samplerManager;
std::unique_ptr<rhi::vulkan::ShaderManager> g_shaderManager;
std::unique_ptr<rhi::vulkan::BindlessDescriptorManager> g_bindlessManager;
std::unique_ptr<rhi::vulkan::PipelineLayoutManager> g_pipelineLayoutManager;
std::unique_ptr<rhi::vulkan::GraphicsPipelineManager> g_graphicsPipelineManager;
std::unique_ptr<rhi::vulkan::SynchronizationManager> g_syncManager;
std::unique_ptr<rhi::vulkan::CommandPool> g_commandPool;

// 渲染资源
GLFWwindow*                               g_window        = nullptr;
VkPipelineCache                           g_pipelineCache = VK_NULL_HANDLE;
rhi::vulkan::VulkanGraphicsPipelineHandle g_pipeline;
rhi::vulkan::VulkanBufferHandle           g_vertexBuffer;
rhi::vulkan::VulkanBufferHandle           g_indexBuffer;
rhi::vulkan::VulkanBufferHandle           g_uniformBuffer;
rhi::vulkan::VulkanPipelineLayoutHandle   g_pipelineLayout;
uint32_t                                  g_indexCount = 0;

// 当前帧资源
std::vector<rhi::vulkan::CommandBuffer> g_commandBuffers;
VkFence g_renderFence = VK_NULL_HANDLE;
VkSemaphore g_imageAvailableSemaphore = VK_NULL_HANDLE;
VkSemaphore g_renderFinishedSemaphore = VK_NULL_HANDLE;
uint32_t g_currentImageIndex = 0;

// 立方体顶点数据
const std::vector<Vertex> g_cubeVertices = {
    // 前面 (z = -1)
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // 0 - 红
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // 1
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // 2
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // 3
    
    // 后面 (z = 1)
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},  // 4 - 绿
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},  // 5
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},  // 6
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},  // 7
    
    // 左面 (x = -1)
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},  // 8 - 蓝
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},  // 9
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},  // 10
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},  // 11
    
    // 右面 (x = 1)
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},  // 12 - 黄
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},  // 13
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},  // 14
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},  // 15
    
    // 上面 (y = 1)
    {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},  // 16 - 紫
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},  // 17
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},  // 18
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}},  // 19
    
    // 下面 (y = -1)
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},  // 20 - 青
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}},  // 21
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},  // 22
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}},  // 23
};

// 立方体索引数据 (每个面2个三角形)
const std::vector<uint32_t> g_cubeIndices = {
    // 前面
    0, 1, 2, 0, 2, 3,
    // 后面
    4, 6, 5, 4, 7, 6,
    // 左面
    8, 9, 10, 8, 10, 11,
    // 右面
    12, 14, 13, 12, 15, 14,
    // 上面
    16, 17, 18, 16, 18, 19,
    // 下面
    20, 22, 21, 20, 23, 22
};

// 创建 GLFW 窗口
GLFWwindow* CreateGLFWWindow(uint32_t width, uint32_t height) {
    if (!glfwInit()) {
        return nullptr;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), 
                           "Engine Cube Render", nullptr, nullptr);
}

// 上传数据到GPU Buffer
void UploadBufferData(rhi::vulkan::VulkanBufferHandle buffer, const void* data, size_t size) {
    auto* bufferPtr = g_bufferManager->get(buffer);
    if (!bufferPtr) return;

    // 如果buffer可映射，直接复制
    if (void* mapped = g_bufferManager->getMappedPtr(buffer)) {
        memcpy(mapped, data, size);
    } else {
        // 否则需要使用staging buffer（简化处理）
        std::cerr << "Warning: Buffer not mappable, data upload skipped" << std::endl;
    }
}

// 初始化RHI系统
bool InitializeRHI() {
    std::cout << "=== Initializing RHI ===" << std::endl;

    // 1. 创建设备管理器
    g_deviceManager = std::make_unique<rhi::vulkan::DeviceManager>();
    try {
        g_deviceManager->Initialize();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize DeviceManager: " << e.what() << std::endl;
        return false;
    }
    std::cout << "✓ DeviceManager initialized" << std::endl;

    VkDevice device = g_deviceManager->GetLogicalDevice();
    VmaAllocator allocator = g_deviceManager->GetAllocator();

    // 2. 创建窗口
    g_window = CreateGLFWWindow(1280, 720);
    if (!g_window) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    HWND hwnd = glfwGetWin32Window(g_window);
    std::cout << "✓ Window created" << std::endl;

    // 3. 创建SwapChain
    g_swapChainManager = std::make_unique<rhi::vulkan::SwapChainManager>(*g_deviceManager);
    rhi::vulkan::SwapChainConfig swapConfig;
    swapConfig.width = 1280;
    swapConfig.height = 720;
    swapConfig.imageCount = 3;
    swapConfig.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapConfig.preferredFormat = VK_FORMAT_B8G8R8A8_UNORM;
    
    g_swapChainManager->Initialize(swapConfig, hwnd);
    std::cout << "✓ SwapChain created" << std::endl;

    // 4. 创建资源管理器
    g_bufferManager = std::make_unique<rhi::vulkan::BufferManager>(device, allocator);
    g_textureManager = std::make_unique<rhi::vulkan::TextureManager>(device, allocator);
    g_samplerManager = std::make_unique<rhi::vulkan::SamplerManager>(device);
    g_shaderManager = std::make_unique<rhi::vulkan::ShaderManager>(device);
    std::cout << "✓ Resource managers created" << std::endl;

    // 5. 创建Bindless Descriptor Manager
    g_bindlessManager = std::make_unique<rhi::vulkan::BindlessDescriptorManager>(device);
    rhi::vulkan::BindlessDescriptorManager::Config bindlessConfig;
    bindlessConfig.maxTextures = 1024;
    bindlessConfig.maxBuffers = 256;
    bindlessConfig.maxSamplers = 64;
    
    if (!g_bindlessManager->initialize(bindlessConfig)) {
        std::cerr << "Failed to initialize BindlessDescriptorManager" << std::endl;
        return false;
    }
    
    g_textureManager->setBindlessManager(g_bindlessManager.get());
    std::cout << "✓ BindlessDescriptorManager initialized" << std::endl;

    // 6. 创建Pipeline Cache
    VkPipelineCacheCreateInfo cacheInfo = {};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkCreatePipelineCache(device, &cacheInfo, nullptr, &g_pipelineCache);
    std::cout << "✓ PipelineCache created" << std::endl;

    // 7. 创建Pipeline管理器
    g_pipelineLayoutManager = std::make_unique<rhi::vulkan::PipelineLayoutManager>(device);
    g_graphicsPipelineManager = std::make_unique<rhi::vulkan::GraphicsPipelineManager>(device, g_pipelineCache);
    std::cout << "✓ Pipeline managers created" << std::endl;

    // 8. 创建同步管理器
    g_syncManager = std::make_unique<rhi::vulkan::SynchronizationManager>(*g_deviceManager);
    std::cout << "✓ SynchronizationManager created" << std::endl;

    // 9. 创建CommandPool
    auto queueFamilies = g_deviceManager->GetQueueFamilyIndices();
    g_commandPool = std::make_unique<rhi::vulkan::CommandPool>(*g_deviceManager, queueFamilies.graphicsFamily);
    
    rhi::vulkan::CommandPoolConfig poolConfig;
    poolConfig.queueFamilyIndex = queueFamilies.graphicsFamily;
    poolConfig.initialBufferCount = 4;
    g_commandPool->Initialize(poolConfig);
    std::cout << "✓ CommandPool created" << std::endl;

    // 10. 创建同步对象
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceInfo, nullptr, &g_renderFence);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &g_imageAvailableSemaphore);
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &g_renderFinishedSemaphore);
    std::cout << "✓ Synchronization objects created" << std::endl;

    std::cout << "=== RHI Initialization Complete ===" << std::endl;
    return true;
}

// 创建立方体几何数据
bool CreateCubeGeometry() {
    std::cout << "\n=== Creating Cube Geometry ===" << std::endl;

    g_indexCount = static_cast<uint32_t>(g_cubeIndices.size());

    // 创建Vertex Buffer (HostVisible便于上传)
    rhi::vulkan::BufferDesc vbDesc;
    vbDesc.size = g_cubeVertices.size() * sizeof(Vertex);
    vbDesc.usage = rhi::vulkan::ResourceUsage::VertexBuffer;
    vbDesc.persistentMap = true;  // 可映射
    
    g_vertexBuffer = g_bufferManager->createBuffer(vbDesc);
    if (!g_vertexBuffer.isValid()) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }
    UploadBufferData(g_vertexBuffer, g_cubeVertices.data(), vbDesc.size);

    // 创建Index Buffer
    rhi::vulkan::BufferDesc ibDesc;
    ibDesc.size = g_cubeIndices.size() * sizeof(uint32_t);
    ibDesc.usage = rhi::vulkan::ResourceUsage::IndexBuffer;
    ibDesc.persistentMap = true;
    
    g_indexBuffer = g_bufferManager->createBuffer(ibDesc);
    if (!g_indexBuffer.isValid()) {
        std::cerr << "Failed to create index buffer" << std::endl;
        return false;
    }
    UploadBufferData(g_indexBuffer, g_cubeIndices.data(), ibDesc.size);

    std::cout << "✓ Cube geometry created" << std::endl;
    std::cout << "  Vertices: " << g_cubeVertices.size() << std::endl;
    std::cout << "  Indices: " << g_cubeIndices.size() << " (" << (g_indexCount / 6) << " triangles)" << std::endl;

    return true;
}

// 全局DescriptorSet和PipelineLayout
VkDescriptorSetLayout g_descriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSet g_descriptorSet = VK_NULL_HANDLE;
VkDescriptorPool g_descriptorPool = VK_NULL_HANDLE;
VkPipelineLayout g_vkPipelineLayout = VK_NULL_HANDLE;

// 创建渲染管线
bool CreateRenderPipeline() {
    std::cout << "\n=== Creating Render Pipeline ===" << std::endl;

    // 1. 加载Shader（从项目根目录查找）
    auto vsHandle = g_shaderManager->loadShader("../../shaders/cube.vert.spv", 
                                                 rhi::vulkan::ShaderStage::Vertex);
    auto fsHandle = g_shaderManager->loadShader("../../shaders/cube.frag.spv",
                                                 rhi::vulkan::ShaderStage::Fragment);
    
    if (!vsHandle.isValid() || !fsHandle.isValid()) {
        std::cerr << "⚠ Warning: Failed to load shaders" << std::endl;
        return true;
    }
    
    std::cout << "✓ Shaders loaded" << std::endl;

    VkDevice device = g_deviceManager->GetLogicalDevice();

    // 2. 创建Uniform Buffer
    rhi::vulkan::BufferDesc ubDesc;
    ubDesc.size = sizeof(UniformBufferObject);
    ubDesc.usage = rhi::vulkan::ResourceUsage::UniformBuffer;
    ubDesc.persistentMap = true;
    
    g_uniformBuffer = g_bufferManager->createBuffer(ubDesc);
    if (!g_uniformBuffer.isValid()) {
        std::cerr << "Failed to create uniform buffer" << std::endl;
        return false;
    }
    std::cout << "✓ UniformBuffer created" << std::endl;

    // 3. 创建DescriptorSetLayout
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &g_descriptorSetLayout));
    std::cout << "✓ DescriptorSetLayout created" << std::endl;

    // 4. 创建PipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &g_descriptorSetLayout;
    
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &g_vkPipelineLayout));
    std::cout << "✓ PipelineLayout created" << std::endl;

    // 5. 创建独立的DescriptorPool用于Uniform Buffer
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &g_descriptorPool));
    
    // 6. 分配DescriptorSet
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &g_descriptorSetLayout;
    
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &g_descriptorSet));
    std::cout << "✓ DescriptorSet allocated" << std::endl;

    // 6. 更新DescriptorSet，绑定Uniform Buffer
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = g_bufferManager->getVkBuffer(g_uniformBuffer);
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);
    
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = g_descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    std::cout << "✓ DescriptorSet updated" << std::endl;

    // 7. 构建GraphicsPipelineDesc
    rhi::vulkan::GraphicsPipelineDesc pipelineDesc;
    
    pipelineDesc.shaders = {
        {g_shaderManager->getVkShaderModule(vsHandle), VK_SHADER_STAGE_VERTEX_BIT, "main"},
        {g_shaderManager->getVkShaderModule(fsHandle), VK_SHADER_STAGE_FRAGMENT_BIT, "main"}
    };
    
    pipelineDesc.vertexInput.bindings = {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
    pipelineDesc.vertexInput.attributes = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
    };
    
    pipelineDesc.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineDesc.layout = g_vkPipelineLayout;
    pipelineDesc.renderTargets.colorFormats = {VK_FORMAT_B8G8R8A8_UNORM};
    
    rhi::vulkan::ColorBlendAttachment blendAttachment;
    blendAttachment.blendEnable = VK_FALSE;
    blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineDesc.colorBlend.attachments = {blendAttachment};
    
    g_pipeline = g_graphicsPipelineManager->createPipeline(pipelineDesc);
    if (!g_pipeline.isValid()) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        return false;
    }
    
    auto* pipelinePtr = g_graphicsPipelineManager->get(g_pipeline);
    if (pipelinePtr) {
        pipelinePtr->layout = g_vkPipelineLayout;
    }
    
    std::cout << "✓ GraphicsPipeline created" << std::endl;
    return true;
}

// 矩阵计算辅助函数
static float g_rotationAngle = 0.0f;

void Mat4Identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Mat4Perspective(float* m, float fov, float aspect, float nearPlane, float farPlane) {
    float f = 1.0f / tanf(fov * 0.5f);
    float nf = 1.0f / (nearPlane - farPlane);
    
    memset(m, 0, 16 * sizeof(float));
    m[0] = f / aspect;   // X缩放（考虑宽高比）
    m[5] = f;            // Y缩放
    m[10] = (farPlane + nearPlane) * nf;  // Z缩放
    m[11] = -1.0f;       // 透视除数
    m[14] = 2.0f * farPlane * nearPlane * nf;  // Z平移
}

void Mat4LookAt(float* m, float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ) {
    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;
    
    // 归一化front
    float flen = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= flen; fy /= flen; fz /= flen;
    
    // 计算right = front x up
    float rx = fy * upZ - fz * upY;
    float ry = fz * upX - fx * upZ;
    float rz = fx * upY - fy * upX;
    
    // 归一化right
    float rlen = sqrtf(rx*rx + ry*ry + rz*rz);
    rx /= rlen; ry /= rlen; rz /= rlen;
    
    // 重新计算up = right x front
    float ux = ry * fz - rz * fy;
    float uy = rz * fx - rx * fz;
    float uz = rx * fy - ry * fx;
    
    // 列主序视图矩阵
    m[0] = rx; m[4] = ry; m[8] = rz;  m[12] = -(rx*eyeX + ry*eyeY + rz*eyeZ);
    m[1] = ux; m[5] = uy; m[9] = uz;  m[13] = -(ux*eyeX + uy*eyeY + uz*eyeZ);
    m[2] = -fx; m[6] = -fy; m[10] = -fz; m[14] = -(-fx*eyeX - fy*eyeY - fz*eyeZ);
    m[3] = 0;  m[7] = 0;  m[11] = 0;   m[15] = 1.0f;
}

void Mat4RotateY(float* m, float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    
    Mat4Identity(m);
    m[0] = cosA;  m[2] = sinA;
    m[8] = -sinA; m[10] = cosA;
}

void Mat4Multiply(float* result, const float* a, const float* b) {
    float temp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i*4 + j] = 
                a[i*4 + 0] * b[0*4 + j] +
                a[i*4 + 1] * b[1*4 + j] +
                a[i*4 + 2] * b[2*4 + j] +
                a[i*4 + 3] * b[3*4 + j];
        }
    }
    memcpy(result, temp, sizeof(temp));
}

void UpdateMVP(float* mvp, float aspect) {
    // 测试：使用恒等矩阵，立方体应该在屏幕中心
    // 顶点范围是 -0.5 到 0.5，直接映射到裁剪空间
    Mat4Identity(mvp);
    
    // 稍微缩小一点，让立方体在屏幕内
    mvp[0] = 0.5f;   // X缩放
    mvp[5] = 0.5f;   // Y缩放
    mvp[10] = 0.5f;  // Z缩放
}

// 录制渲染命令
void RecordRenderCommands(rhi::vulkan::CommandBuffer& cmd, uint32_t imageIndex) {
    VkCommandBuffer vkCmd = cmd.GetHandle();
    
    // 获取swapchain image view
    const auto& swapImages = g_swapChainManager->GetImages();
    if (imageIndex >= swapImages.size()) return;
    
    VkImageView currentView = swapImages[imageIndex].view;
    
    // Begin Rendering (Dynamic Rendering)
    VkRenderingAttachmentInfo colorAttachment = {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = currentView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {0.1f, 0.1f, 0.15f, 1.0f};  // 深色背景

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, {1280, 720}};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(vkCmd, &renderingInfo);

    // 设置Viewport和Scissor
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 1280.0f;
    viewport.height = 720.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(vkCmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {1280, 720};
    vkCmdSetScissor(vkCmd, 0, 1, &scissor);

    // 如果pipeline有效，绘制立方体
    if (g_pipeline.isValid()) {
        
        VkPipeline vkPipeline = g_graphicsPipelineManager->getVkPipeline(g_pipeline);
        vkCmdBindPipeline(vkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        // 绑定Vertex Buffer
        VkBuffer vertexBuffer = g_bufferManager->getVkBuffer(g_vertexBuffer);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(vkCmd, 0, 1, &vertexBuffer, offsets);

        // 绑定Index Buffer
        VkBuffer indexBuffer = g_bufferManager->getVkBuffer(g_indexBuffer);
        vkCmdBindIndexBuffer(vkCmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        // 更新MVP矩阵到Uniform Buffer
        float mvp[16];
        UpdateMVP(mvp, 1280.0f / 720.0f);
        
        void* mappedData = g_bufferManager->getMappedPtr(g_uniformBuffer);
        if (mappedData) {
            memcpy(mappedData, mvp, sizeof(mvp));
        }
        
        // 绑定DescriptorSet
        vkCmdBindDescriptorSets(vkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                g_vkPipelineLayout, 0, 1, &g_descriptorSet, 0, nullptr);

        // 绘制
        vkCmdDrawIndexed(vkCmd, g_indexCount, 1, 0, 0, 0);
        
        // 更新旋转角度
        g_rotationAngle += 0.02f;
    }

    vkCmdEndRendering(vkCmd);
    
    // 转换SwapChain Image Layout到PRESENT_SRC_KHR（用于呈现）
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    // 使用当前swapchain image
    if (imageIndex < swapImages.size()) {
        barrier.image = swapImages[imageIndex].image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        
        vkCmdPipelineBarrier(vkCmd, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
    }
}

// 渲染单帧
void RenderFrame() {
    VkDevice device = g_deviceManager->GetLogicalDevice();

    // 1. 等待前一帧完成
    vkWaitForFences(device, 1, &g_renderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &g_renderFence);

    // 2. 获取下一帧image
    VkSwapchainKHR swapchain = g_swapChainManager->GetSwapChain();
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 
                                            g_imageAvailableSemaphore, VK_NULL_HANDLE, 
                                            &g_currentImageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    // 3. 录制命令缓冲
    auto cmd = g_commandPool->Allocate();
    cmd.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    RecordRenderCommands(cmd, g_currentImageIndex);
    cmd.End();

    // 4. 提交命令
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {g_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer cmdBuffer = cmd.GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    VkSemaphore signalSemaphores[] = {g_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkQueue graphicsQueue = g_deviceManager->GetGraphicsQueue();
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, g_renderFence);

    // 5. Present
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &g_currentImageIndex;

    VkQueue presentQueue = g_deviceManager->GetGraphicsQueue();
    vkQueuePresentKHR(presentQueue, &presentInfo);

    // 释放命令缓冲（RAII会自动处理）
    cmd = rhi::vulkan::CommandBuffer();
}

// 主渲染循环
void RunMainLoop() {
    std::cout << "\n=== Starting Render Loop ===" << std::endl;
    std::cout << "Press ESC to exit\n" << std::endl;

    uint32_t frameCount = 0;
    auto lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(g_window)) {
        glfwPollEvents();

        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        RenderFrame();
        frameCount++;

        auto currentTime = glfwGetTime();
        if (currentTime - lastTime >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }
    }

    // 等待设备空闲
    vkDeviceWaitIdle(g_deviceManager->GetLogicalDevice());
    std::cout << "\n=== Render Loop Ended ===" << std::endl;
}

// 清理资源
void ShutdownRHI() {
    std::cout << "\n=== Shutting Down RHI ===" << std::endl;

    if (g_deviceManager) {
        vkDeviceWaitIdle(g_deviceManager->GetLogicalDevice());
    }

    // 销毁同步对象
    VkDevice device = g_deviceManager ? g_deviceManager->GetLogicalDevice() : VK_NULL_HANDLE;
    if (device != VK_NULL_HANDLE) {
        if (g_renderFence != VK_NULL_HANDLE) vkDestroyFence(device, g_renderFence, nullptr);
        if (g_imageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, g_imageAvailableSemaphore, nullptr);
        if (g_renderFinishedSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(device, g_renderFinishedSemaphore, nullptr);
    }

    // 清理Vulkan对象
    if (device != VK_NULL_HANDLE) {
        if (g_vkPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, g_vkPipelineLayout, nullptr);
        }
        if (g_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, g_descriptorSetLayout, nullptr);
        }
        if (g_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, g_descriptorPool, nullptr);
        }
        if (g_pipelineCache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(device, g_pipelineCache, nullptr);
        }
    }

    // 释放资源
    g_commandPool.reset();
    g_syncManager.reset();
    g_graphicsPipelineManager.reset();
    g_pipelineLayoutManager.reset();
    g_bindlessManager.reset();
    g_shaderManager.reset();
    g_samplerManager.reset();
    g_textureManager.reset();
    g_bufferManager.reset();

    g_swapChainManager.reset();

    if (g_window) {
        glfwDestroyWindow(g_window);
        glfwTerminate();
    }

    g_deviceManager.reset();

    std::cout << "=== RHI Shutdown Complete ===" << std::endl;
}

} // namespace engine

void engine_init() {
    std::cout << "========================================" << std::endl;
    std::cout << "    Engine Cube Render Demo" << std::endl;
    std::cout << "========================================\n" << std::endl;

    try {
        if (!engine::InitializeRHI()) {
            throw std::runtime_error("RHI initialization failed");
        }

        if (!engine::CreateCubeGeometry()) {
            throw std::runtime_error("Geometry creation failed");
        }

        if (!engine::CreateRenderPipeline()) {
            throw std::runtime_error("Pipeline creation failed");
        }

        engine::RunMainLoop();
        engine::ShutdownRHI();

        std::cout << "\nEngine exited successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        engine::ShutdownRHI();
        throw;
    }
}

int main() {
    std::cout << "Engine starting...\n" << std::endl;

    try {
        engine_init();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Engine failed: " << e.what() << std::endl;
        return -1;
    }
}
