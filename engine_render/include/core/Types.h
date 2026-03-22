#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <span>

namespace render {

// 基础整数类型
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// 句柄类型
using Handle = u32;
static constexpr Handle InvalidHandle = 0;

// 基础常量
static constexpr u32 MaxRenderTargets = 8;
static constexpr u32 MaxVertexAttributes = 16;
static constexpr u32 MaxVertexBindings = 4;
static constexpr u32 MaxDescriptorSets = 4;
static constexpr u32 MaxDescriptorsPerSet = 32;
static constexpr u32 MaxPushConstantsSize = 128;
static constexpr u32 MaxMipLevels = 16;
static constexpr u32 MaxArrayLayers = 2048;

/**
 * @brief 渲染API类型
 * @details 支持的底层图形API
 */
enum class RenderAPI {
    Vulkan,     ///< Vulkan (跨平台，Windows/Linux/Android)
    DirectX12,  ///< DirectX 12 (Windows/Xbox)
    Metal,      ///< Metal (macOS/iOS)
    OpenGL,     ///< OpenGL (传统支持)
    Count       ///< API类型数量
};

/**
 * @brief GPU资源类型
 * @details 区分不同类型的GPU资源
 */
enum class ResourceType {
    Texture1D,          ///< 1D纹理
    Texture2D,          ///< 2D纹理
    Texture3D,          ///< 3D体积纹理
    TextureCube,        ///< 立方体贴图
    Texture1DArray,     ///< 1D纹理数组
    Texture2DArray,     ///< 2D纹理数组
    TextureCubeArray,   ///< 立方体贴图数组
    Buffer,             ///< 缓冲区
    Count               ///< 资源类型数量
};

/**
 * @brief 纹理维度类型
 * @details 定义纹理的维度和数组类型
 */
enum class TextureType {
    Texture1D,          ///< 1D纹理
    Texture2D,          ///< 2D纹理（最常见）
    Texture3D,          ///< 3D体积纹理
    TextureCube,        ///< 立方体贴图（6面）
    Texture1DArray,     ///< 1D纹理数组
    Texture2DArray,     ///< 2D纹理数组
    TextureCubeArray,   ///< 立方体贴图数组
    Count               ///< 纹理类型数量
};

/**
 * @brief 缓冲区用途类型
 * @details 定义缓冲区的具体用途
 */
enum class BufferType {
    Default,    ///< 通用缓冲区
    Vertex,     ///< 顶点缓冲区
    Index,      ///< 索引缓冲区
    Uniform,    ///< 统一缓冲区(UBO/Constant Buffer)
    Storage,    ///< 存储缓冲区(SSBO)
    Indirect,   ///< 间接绘制缓冲区
    Count       ///< 缓冲区类型数量
};

/**
 * @brief 资源状态
 * @details 定义资源在GPU管线中的访问状态，用于自动屏障管理
 * @note 这些状态可以组合使用（位掩码）
 */
enum class ResourceState : u32 {
    Undefined = 0,              ///< 未定义状态（初始状态）
    Common = 1 << 0,            ///< 通用状态（同时读写）
    VertexBuffer = 1 << 1,      ///< 作为顶点缓冲区绑定
    IndexBuffer = 1 << 2,       ///< 作为索引缓冲区绑定
    ConstantBuffer = 1 << 3,    ///< 作为常量缓冲区绑定
    RenderTarget = 1 << 4,      ///< 作为渲染目标（颜色附件）
    DepthWrite = 1 << 5,        ///< 深度缓冲区可写
    DepthRead = 1 << 6,         ///< 深度缓冲区只读
    ShaderResource = 1 << 7,    ///< 着色器资源（纹理/采样器）
    UnorderedAccess = 1 << 8,   ///< 无序访问视图（UAV/RW资源）
    CopySource = 1 << 9,        ///< 复制操作源
    CopyDest = 1 << 10,         ///< 复制操作目标
    Present = 1 << 11,          ///< 可呈现到屏幕
    ResolveSource = 1 << 12,    ///< MSAA解析源
    ResolveDest = 1 << 13,      ///< MSAA解析目标
    AccelerationStructure = 1 << 14,  ///< 光线追踪加速结构
    Count                       ///< 状态数量
};

inline ResourceState operator|(ResourceState a, ResourceState b) {
    return static_cast<ResourceState>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline ResourceState operator&(ResourceState a, ResourceState b) {
    return static_cast<ResourceState>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief 像素格式枚举
 * @details 支持各种纹理和渲染目标的像素格式
 */
enum class Format {
    Unknown,            ///< 未知格式

    // 8-bit 单通道格式
    R8_UNORM,           ///< 8位无归一化R通道
    R8_SNORM,           ///< 8位有归一化R通道
    R8_UINT,            ///< 8位无符号整数R通道
    R8_SINT,            ///< 8位有符号整数R通道

    // 8-bit 双通道格式
    RG8_UNORM,          ///< 8位RG无归一化
    RG8_SNORM,          ///< 8位RG有归一化
    RG8_UINT,           ///< 8位RG无符号整数
    RG8_SINT,           ///< 8位RG有符号整数

    // 8-bit 四通道格式
    RGBA8_UNORM,        ///< 8位RGBA无归一化（标准颜色格式）
    RGBA8_SNORM,        ///< 8位RGBA有归一化
    RGBA8_UINT,         ///< 8位RGBA无符号整数
    RGBA8_SINT,         ///< 8位RGBA有符号整数
    RGBA8_SRGB,         ///< 8位RGBA sRGB颜色空间
    BGRA8_UNORM,        ///< 8位BGRA无归一化（DirectX标准）
    BGRA8_SRGB,         ///< 8位BGRA sRGB颜色空间

    // 16-bit 单通道格式
    R16_UNORM,          ///< 16位无归一化R通道
    R16_SNORM,          ///< 16位有归一化R通道
    R16_UINT,           ///< 16位无符号整数R通道
    R16_SINT,           ///< 16位有符号整数R通道
    R16_FLOAT,          ///< 16位浮点R通道（半精度）

    // 16-bit 双通道格式
    RG16_UNORM,         ///< 16位RG无归一化
    RG16_SNORM,         ///< 16位RG有归一化
    RG16_UINT,          ///< 16位RG无符号整数
    RG16_SINT,          ///< 16位RG有符号整数
    RG16_FLOAT,         ///< 16位RG浮点

    // 16-bit 四通道格式
    RGBA16_UNORM,       ///< 16位RGBA无归一化
    RGBA16_SNORM,       ///< 16位RGBA有归一化
    RGBA16_UINT,        ///< 16位RGBA无符号整数
    RGBA16_SINT,        ///< 16位RGBA有符号整数
    RGBA16_FLOAT,       ///< 16位RGBA浮点（HDR常用）

    // 32-bit 单通道格式
    R32_UINT,           ///< 32位无符号整数R通道
    R32_SINT,           ///< 32位有符号整数R通道
    R32_FLOAT,          ///< 32位浮点R通道（单精度）

    // 32-bit 双通道格式
    RG32_UINT,          ///< 32位RG无符号整数
    RG32_SINT,          ///< 32位RG有符号整数
    RG32_FLOAT,         ///< 32位RG浮点

    // 32-bit 三通道格式
    RGB32_UINT,         ///< 32位RGB无符号整数
    RGB32_SINT,         ///< 32位RGB有符号整数
    RGB32_FLOAT,        ///< 32位RGB浮点

    // 32-bit 四通道格式
    RGBA32_UINT,        ///< 32位RGBA无符号整数
    RGBA32_SINT,        ///< 32位RGBA有符号整数
    RGBA32_FLOAT,       ///< 32位RGBA浮点（高精度HDR）

    // 深度/模板格式
    D16_UNORM,          ///< 16位深度无归一化
    D24_UNORM_S8_UINT,  ///< 24位深度+8位模板（常见）
    D32_FLOAT,          ///< 32位浮点深度
    D32_FLOAT_S8X24_UINT, ///< 32位浮点深度+8位模板+24位未使用

    // 压缩格式 (BC/DXT)
    BC1_UNORM,          ///< BC1/DXT1压缩（无Alpha）
    BC1_SRGB,           ///< BC1 sRGB颜色空间
    BC3_UNORM,          ///< BC3/DXT5压缩（有Alpha）
    BC3_SRGB,           ///< BC3 sRGB颜色空间
    BC5_UNORM,          ///< BC5压缩（双通道，常用于法线贴图）
    BC5_SNORM,          ///< BC5有归一化
    BC7_UNORM,          ///< BC7高质量压缩（支持Alpha）
    BC7_SRGB,           ///< BC7 sRGB颜色空间
    Count               ///< 格式数量
};

/**
 * @brief 资源用途标志
 * @details 创建资源时指定其用途（位掩码，可组合）
 */
enum class ResourceUsage : u32 {
    None = 0,                   ///< 无特定用途
    ShaderResource = 1 << 0,    ///< 着色器资源（SRV/纹理）
    RenderTarget = 1 << 2,      ///< 渲染目标（颜色附件）
    DepthStencil = 1 << 3,      ///< 深度模板附件
    UnorderedAccess = 1 << 4,   ///< 无序访问（UAV/RW）
    CopySource = 1 << 5,        ///< 复制操作源
    CopyDest = 1 << 6,          ///< 复制操作目标
    VertexBuffer = 1 << 7,      ///< 顶点缓冲区
    IndexBuffer = 1 << 8,       ///< 索引缓冲区
    ConstantBuffer = 1 << 9,        ///< 常量/统一缓冲区
    StorageBuffer = 1 << 10,        ///< 存储缓冲区（SSBO）
    IndirectBuffer = 1 << 11,       ///< 间接绘制缓冲区
    TransferSrc = 1 << 12,          ///< 传输操作源
    TransferDst = 1 << 13,          ///< 传输操作目标
    Sampled = 1 << 14,              ///< 可采样纹理
    InputAttachment = 1 << 15,      ///< 输入附件（Subpass输入）
    ShadingRate = 1 << 16,          ///< 可变着色率附件
    AccelerationStructure = 1 << 17, ///< 光线追踪加速结构
    Count                           ///< 用途数量
};

inline ResourceUsage operator|(ResourceUsage a, ResourceUsage b) {
    return static_cast<ResourceUsage>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline ResourceUsage operator&(ResourceUsage a, ResourceUsage b) {
    return static_cast<ResourceUsage>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasUsage(ResourceUsage flags, ResourceUsage usage) {
    return (static_cast<u32>(flags) & static_cast<u32>(usage)) != 0;
}

/**
 * @brief 内存类型
 * @details 定义资源的内存分配位置和访问模式
 */
enum class MemoryType {
    Default,        ///< GPU独占内存（GPU读写最快）
    Upload,         ///< 上传堆（CPU写，GPU读）
    Readback,       ///< 回读堆（GPU写，CPU读）
    DeviceUpload,   ///< 设备本地可访问（BAR内存，PCIe直接访问）
    Count           ///< 内存类型数量
};

/**
 * @brief 着色器阶段
 * @details 定义图形和计算管线中的着色器阶段（位掩码）
 */
enum class ShaderStage : u32 {
    None = 0,               ///< 无着色器阶段
    Vertex = 1 << 0,        ///< 顶点着色器
    Hull = 1 << 1,          ///< 外壳/曲面细分控制着色器
    Domain = 1 << 2,        ///< 域/曲面细分评估着色器
    Geometry = 1 << 3,      ///< 几何着色器
    Pixel = 1 << 4,         ///< 像素/片段着色器
    Compute = 1 << 5,       ///< 计算着色器
    RayGen = 1 << 6,        ///< 光线生成着色器（光追）
    AnyHit = 1 << 7,        ///< 任意命中着色器（光追）
    ClosestHit = 1 << 8,    ///< 最近命中着色器（光追）
    Miss = 1 << 9,          ///< 未命中着色器（光追）
    Intersection = 1 << 10, ///< 交集着色器（光追）
    Callable = 1 << 11,     ///< 可调用着色器（光追）
    All = 0xFFFFFFFF,       ///< 所有阶段
    AllGraphics = Vertex | Hull | Domain | Geometry | Pixel,  ///< 所有图形阶段
    AllRayTracing = RayGen | AnyHit | ClosestHit | Miss | Intersection | Callable  ///< 所有光追阶段
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline ShaderStage operator&(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief 图元拓扑类型
 * @details 定义顶点如何组装成图元
 */
enum class PrimitiveTopology {
    PointList,          ///< 点列表（每个顶点一个点）
    LineList,           ///< 线段列表（每两个顶点一条线）
    LineStrip,          ///< 线段带（连续线段）
    TriangleList,       ///< 三角形列表（每三个顶点一个三角形，最常见）
    TriangleStrip,      ///< 三角形带（共享边的连续三角形）
    TriangleFan,        ///< 三角形扇（共享第一个顶点）
    LineListAdj,        ///< 带邻接信息的线段列表
    LineStripAdj,       ///< 带邻接信息的线段带
    TriangleListAdj,    ///< 带邻接信息的三角形列表
    TriangleStripAdj,   ///< 带邻接信息的三角形带
    PatchList1,         ///< 1控制点的曲面细分面片
    PatchList2,         ///< 2控制点的曲面细分面片
    PatchList3,         ///< 3控制点的曲面细分面片
    PatchList4,         ///< 4控制点的曲面细分面片
    PatchList5,         ///< 5控制点的曲面细分面片
    PatchList6,         ///< 6控制点的曲面细分面片
    PatchList7,         ///< 7控制点的曲面细分面片
    PatchList8,         ///< 8控制点的曲面细分面片
    PatchList9,         ///< 9控制点的曲面细分面片
    PatchList10,        ///< 10控制点的曲面细分面片
    PatchList11,        ///< 11控制点的曲面细分面片
    PatchList12,        ///< 12控制点的曲面细分面片
    PatchList13,        ///< 13控制点的曲面细分面片
    PatchList14,        ///< 14控制点的曲面细分面片
    PatchList15,        ///< 15控制点的曲面细分面片
    PatchList16,        ///< 16控制点的曲面细分面片
    PatchList17,        ///< 17控制点的曲面细分面片
    PatchList18,        ///< 18控制点的曲面细分面片
    PatchList19,        ///< 19控制点的曲面细分面片
    PatchList20,        ///< 20控制点的曲面细分面片
    PatchList21,        ///< 21控制点的曲面细分面片
    PatchList22,        ///< 22控制点的曲面细分面片
    PatchList23,        ///< 23控制点的曲面细分面片
    PatchList24,        ///< 24控制点的曲面细分面片
    PatchList25,        ///< 25控制点的曲面细分面片
    PatchList26,        ///< 26控制点的曲面细分面片
    PatchList27,        ///< 27控制点的曲面细分面片
    PatchList28,        ///< 28控制点的曲面细分面片
    PatchList29,        ///< 29控制点的曲面细分面片
    PatchList30,        ///< 30控制点的曲面细分面片
    PatchList31,        ///< 31控制点的曲面细分面片
    PatchList32,
    Count
};

// 填充模式
/**
 * @brief 填充模式
 * @details 定义图元的填充方式
 */
enum class FillMode {
    Wireframe,  ///< 线框模式
    Solid,      ///< 实心填充模式
    Count       ///< 填充模式数量
};

/**
 * @brief 裁剪模式
 * @details 定义面剔除方式
 */
enum class CullMode {
    None,   ///< 不剔除
    Front,  ///< 剔除正面
    Back,   ///< 剔除背面（最常见）
    Count   ///< 裁剪模式数量
};

/**
 * @brief 比较函数
 * @details 用于深度测试和模板测试的比较操作
 */
enum class ComparisonFunc {
    Never,          ///< 永远不通过
    Less,           ///< 小于（深度测试常用）
    Equal,          ///< 等于
    LessEqual,      ///< 小于等于（深度测试常用）
    Greater,        ///< 大于
    NotEqual,       ///< 不等于
    GreaterEqual,   ///< 大于等于
    Always,         ///< 总是通过
    Count           ///< 比较函数数量
};

/**
 * @brief 混合因子
 * @details 颜色混合时使用的源/目标因子
 */
enum class BlendFactor {
    Zero,                   ///< 0
    One,                    ///< 1
    SrcColor,               ///< 源颜色
    OneMinusSrcColor,       ///< 1 - 源颜色
    SrcAlpha,               ///< 源Alpha
    OneMinusSrcAlpha,       ///< 1 - 源Alpha（常用透明混合）
    DstColor,               ///< 目标颜色
    OneMinusDstColor,       ///< 1 - 目标颜色
    DstAlpha,               ///< 目标Alpha
    OneMinusDstAlpha,       ///< 1 - 目标Alpha
    SrcAlphaSaturate,       ///< 饱和源Alpha (min(源Alpha, 1-目标Alpha))
    BlendColor,             ///< 混合颜色（常量）
    OneMinusBlendColor,     ///< 1 - 混合颜色
    Src1Color,              ///< 第二源颜色（双源混合）
    OneMinusSrc1Color,      ///< 1 - 第二源颜色
    Src1Alpha,              ///< 第二源Alpha
    OneMinusSrc1Alpha,      ///< 1 - 第二源Alpha
    Count                   ///< 混合因子数量
};

/**
 * @brief 混合操作
 * @details 颜色混合时使用的数学操作
 */
enum class BlendOp {
    Add,                ///< 相加（源 * 因子 + 目标 * 因子）
    Subtract,           ///< 相减（源 * 因子 - 目标 * 因子）
    ReverseSubtract,    ///< 反向相减（目标 * 因子 - 源 * 因子）
    Min,                ///< 取最小值
    Max,                ///< 取最大值
    Count               ///< 混合操作数量
};

/**
 * @brief 模板操作
 * @details 模板测试通过/失败时对模板缓冲区的操作
 */
enum class StencilOp {
    Keep,           ///< 保持当前值
    Zero,           ///< 设为0
    Replace,        ///< 替换为参考值
    IncrementSat,   ///< 递增（饱和到最大值）
    DecrementSat,   ///< 递减（饱和到0）
    Invert,         ///< 按位取反
    Increment,      ///< 递增（溢出环绕）
    Decrement,      ///< 递减（下溢环绕）
    Count           ///< 模板操作数量
};

/**
 * @brief 纹理过滤模式
 * @details 纹理采样时使用的过滤方式
 */
enum class Filter {
    Nearest,                ///< 最近邻过滤（锐利，有锯齿）
    Linear,                 ///< 线性过滤（平滑，模糊）
    NearestMipmapNearest,   ///< 最近Mip层级，最近过滤
    NearestMipmapLinear,    ///< 线性插值Mip层级，最近过滤
    LinearMipmapNearest,    ///< 最近Mip层级，线性过滤
    LinearMipmapLinear,     ///< 三线性过滤（最佳质量）
    Anisotropic,            ///< 各向异性过滤（斜视角优化）
    Count                   ///< 过滤模式数量
};

/**
 * @brief 纹理寻址模式
 * @details UV坐标超出[0,1]范围时的处理方式
 */
enum class AddressMode {
    Wrap,       ///< 重复环绕（0.9 -> 0.1）
    Mirror,     ///< 镜像重复（0.9 -> 0.1反向）
    Clamp,      ///< 钳制到边缘（0.9 -> 1.0）
    Border,     ///< 使用边界颜色
    MirrorOnce, ///< 镜像一次然后钳制
    Count       ///< 寻址模式数量
};

/**
 * @brief GPU队列类型
 * @details 不同类型的GPU命令队列
 */
enum class QueueType {
    Graphics,       ///< 图形队列（支持所有操作）
    Compute,        ///< 异步计算队列
    Transfer,       ///< 异步传输队列
    SparseBinding,  ///< 稀疏绑定队列
    VideoDecode,    ///< 视频解码队列
    VideoEncode,    ///< 视频编码队列
    Count           ///< 队列类型数量
};

// 队列家族索引
static constexpr u32 InvalidQueueFamily = ~0u;  ///< 无效队列家族索引

/**
 * @brief 队列优先级
 * @details GPU命令队列的调度优先级
 */
enum class QueuePriority : u8 {
    Low = 0,        ///< 低优先级
    Normal = 1,     ///< 正常优先级（默认）
    High = 2,       ///< 高优先级
    Realtime = 3    ///< 实时优先级（可能导致其他任务饥饿）
};

// 队列创建信息
struct QueueCreateInfo {
    QueueType type = QueueType::Graphics;
    u32 familyIndex = InvalidQueueFamily;
    u32 count = 1;
    QueuePriority priority = QueuePriority::Normal;
};

// 队列同步点
struct QueueSyncPoint {
    QueueType srcQueue = QueueType::Graphics;
    QueueType dstQueue = QueueType::Graphics;
    u64 waitValue = 0;
    bool isExternal = false;  // 是否跨帧同步
};

// Subpass描述
struct SubpassDesc {
    std::vector<u32> inputAttachments;
    std::vector<u32> colorAttachments;
    std::optional<u32> depthStencilAttachment;
    std::vector<u32> preserveAttachments;
    bool hasResolveAttachments = false;
};

/**
 * @brief 管线阶段标志
 * @details GPU管线中的各个执行阶段，用于屏障同步
 */
enum class PipelineStage : u32 {
    None = 0,                       ///< 无阶段
    TopOfPipe = 1 << 0,             ///< 管线起始
    DrawIndirect = 1 << 1,          ///< 间接绘制命令读取
    VertexInput = 1 << 2,           ///< 顶点输入组装
    VertexShader = 1 << 3,          ///< 顶点着色器执行
    TessellationControl = 1 << 4,   ///< 曲面细分控制着色器
    TessellationEvaluation = 1 << 5, ///< 曲面细分评估着色器
    GeometryShader = 1 << 6,        ///< 几何着色器执行
    FragmentShader = 1 << 7,        ///< 片段着色器执行
    EarlyFragmentTests = 1 << 8,    ///< 早期片段测试（深度/模板）
    LateFragmentTests = 1 << 9,     ///< 晚期片段测试
    ColorAttachmentOutput = 1 << 10, ///< 颜色附件输出
    ComputeShader = 1 << 11,        ///< 计算着色器执行
    Transfer = 1 << 12,             ///< 传输操作
    BottomOfPipe = 1 << 13,         ///< 管线结束
    Host = 1 << 14,                 ///< 主机(CPU)访问
    AllGraphics = 1 << 15,          ///< 所有图形阶段
    AllCommands = 1 << 16,          ///< 所有命令
    RayTracing = 1 << 17,           ///< 光线追踪着色器
    AccelerationStructureBuild = 1 << 18, ///< 加速结构构建
    ShadingRateImage = 1 << 19,     ///< 着色率图像
    TaskShader = 1 << 20,           ///< 任务着色器（网格管线）
    MeshShader = 1 << 21,           ///< 网格着色器（网格管线）
};

inline PipelineStage operator|(PipelineStage a, PipelineStage b) {
    return static_cast<PipelineStage>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline PipelineStage operator&(PipelineStage a, PipelineStage b) {
    return static_cast<PipelineStage>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief 内存访问标志
 * @details 定义资源访问类型，用于屏障同步
 */
enum class AccessFlags : u32 {
    None = 0,                           ///< 无访问
    IndirectCommandRead = 1 << 0,       ///< 间接命令读取
    IndexRead = 1 << 1,                 ///< 索引缓冲区读取
    VertexAttributeRead = 1 << 2,       ///< 顶点属性读取
    UniformRead = 1 << 3,               ///< 统一/常量缓冲区读取
    InputAttachmentRead = 1 << 4,       ///< 输入附件读取
    ShaderRead = 1 << 5,                ///< 着色器资源读取
    ShaderWrite = 1 << 6,               ///< 着色器资源写入
    ColorAttachmentRead = 1 << 7,       ///< 颜色附件读取
    ColorAttachmentWrite = 1 << 8,      ///< 颜色附件写入
    DepthStencilAttachmentRead = 1 << 9,    ///< 深度模板附件读取
    DepthStencilAttachmentWrite = 1 << 10,  ///< 深度模板附件写入
    TransferRead = 1 << 11,             ///< 传输操作读取
    TransferWrite = 1 << 12,            ///< 传输操作写入
    HostRead = 1 << 13,                 ///< 主机(CPU)读取
    HostWrite = 1 << 14,                ///< 主机(CPU)写入
    MemoryRead = 1 << 15,               ///< 通用内存读取
    MemoryWrite = 1 << 16,              ///< 通用内存写入
    AccelerationStructureRead = 1 << 17,    ///< 加速结构读取
    AccelerationStructureWrite = 1 << 18,   ///< 加速结构写入
    ShadingRateImageRead = 1 << 19,     ///< 着色率图像读取
};

inline AccessFlags operator|(AccessFlags a, AccessFlags b) {
    return static_cast<AccessFlags>(
        static_cast<u32>(a) | static_cast<u32>(b));
}

inline AccessFlags operator&(AccessFlags a, AccessFlags b) {
    return static_cast<AccessFlags>(
        static_cast<u32>(a) & static_cast<u32>(b));
}

// Subpass依赖
struct SubpassDependency {
    u32 srcSubpass = ~0u;  // ~0u 表示外部子通道
    u32 dstSubpass = ~0u;
    PipelineStage srcStageMask = PipelineStage::TopOfPipe;
    PipelineStage dstStageMask = PipelineStage::BottomOfPipe;
    AccessFlags srcAccessMask = AccessFlags::None;
    AccessFlags dstAccessMask = AccessFlags::None;
    bool byRegion = false;  // 区域依赖（TBR优化）
};

// 图像布局（在AttachmentDesc之前定义）
/**
 * 图像布局（Image Layout）
 * 描述纹理在内存中的排列方式和访问权限，对应 Vulkan 的 VkImageLayout
 * 每种布局优化了特定类型的访问，转换布局会产生隐式屏障
 */
enum class ImageLayout {
    Undefined,              ///< 未定义布局：数据内容未知，不能用于任何访问（除了作为转换的初始状态）
    General,                ///< 通用布局：支持所有操作但性能不是最优，用作回退方案
    ColorAttachment,        ///< 颜色附件：用于片元着色器写入颜色数据（渲染目标）
    DepthStencilAttachment, ///< 深度模板附件：用于深度/模板测试和写入
    DepthStencilReadOnly,   ///< 深度模板只读：用于深度/模板只读访问（如阴影贴图采样）
    ShaderReadOnly,         ///< 着色器只读：用于着色器中的采样读取（SRV）
    TransferSrc,            ///< 传输源：用于复制/解析操作的源纹理
    TransferDst,            ///< 传输目标：用于复制/解析/清除操作的目标纹理
    Preinitialized,         ///< 预初始化：初始数据加载时的布局（仅用于上传初始数据）
    PresentSrc,             ///< 呈现源：用于提交到交换链进行屏幕显示
    SharedPresent,          ///< 共享呈现：用于多队列/多物理设备共享的呈现纹理
    ShadingRateOptimal,     ///< 可变速率着色最优：用于 VRS 的速率图（Vulkan Extension）
    FragmentDensityMapOptimal,///< 片段密度图最优：用于碎片密度图的存储（Vulkan Extension）
    DepthReadOnlyStencilAttachment,   ///< 深度只读 + 模板附件：深度只读，模板可写
    DepthAttachmentStencilReadOnly,   ///< 深度附件 + 模板只读：深度可写，模板只读
    DepthAttachment,        ///< 深度附件：仅深度可写（分离深度/模板时使用）
    DepthReadOnly,          ///< 深度只读：仅深度可读
    StencilAttachment,      ///< 模板附件：仅模板可写（分离深度/模板时使用）
    StencilReadOnly,        ///< 模板只读：仅模板可读
    ReadOnly,               ///< 只读：通用的只读布局（Vulkan 1.3+）
    Attachment,             ///< 附件：通用的附件布局（Vulkan 1.3+）
};

/**
 * @brief 命令缓冲区级别
 * @details 主命令缓冲区可直接提交，次命令缓冲区用于子记录
 */
enum class CommandBufferLevel {
    Primary,    ///< 主命令缓冲区（可直接提交执行）
    Secondary,  ///< 次命令缓冲区（需通过主命令缓冲区执行）
    Count       ///< 级别数量
};

/**
 * @brief 描述符类型
 * @details 着色器资源绑定类型
 */
enum class DescriptorType {
    Sampler,                ///< 采样器
    CombinedImageSampler,   ///< 组合图像采样器
    SampledImage,           ///< 采样图像（纹理）
    StorageImage,
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,   ///< 动态存储缓冲区
    InputAttachment,        ///< 输入附件（Subpass读取）
    AccelerationStructure,  ///< 光线追踪加速结构
    Count                   ///< 描述符类型数量
};

/**
 * @brief 资源视图类型
 * @details 定义资源视图的用途
 */
enum class ResourceViewType {
    ShaderResource,     ///< 着色器资源视图（SRV/采样）
    RenderTarget,       ///< 渲染目标视图（RTV/颜色附件）
    DepthStencil,       ///< 深度模板视图（DSV）
    UnorderedAccess,    ///< 无序访问视图（UAV/RW）
    Count               ///< 视图类型数量
};

/**
 * @brief 索引类型
 * @details 索引缓冲区元素类型
 */
enum class IndexType {
    Uint16, ///< 16位无符号整数索引
    Uint32, ///< 32位无符号整数索引（支持更多顶点）
    Count   ///< 索引类型数量
};

/**
 * @brief 内存映射类型
 * @details 缓冲区内存的CPU访问模式
 */
enum class MapType {
    Read,           ///< 只读访问
    Write,          ///< 只写访问
    ReadWrite,      ///< 读写访问
    WriteDiscard,   ///< 写入并丢弃之前的内容（优化）
    Count           ///< 映射类型数量
};

// 清除值
struct ClearValue {
    union {
        float color[4];
        struct DepthStencil {
            float depth;
            u32 stencil;
        } ds;
    };

    ClearValue() {
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
    }

    ClearValue(float r, float g, float b, float a) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
    }

    static ClearValue depthStencil(float depthValue, u32 stencilValue = 0) {
        ClearValue cv;
        cv.ds.depth = depthValue;
        cv.ds.stencil = stencilValue;
        return cv;
    }
};

// 资源范围
struct ResourceExtent {
    u32 width = 1;
    u32 height = 1;
    u32 depth = 1;

    ResourceExtent() = default;
    ResourceExtent(u32 w, u32 h, u32 d = 1)
        : width(w), height(h), depth(d) {}
};

// 视口
struct Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;

    Viewport() = default;
    Viewport(float x, float y, float w, float h, float minD = 0.0f, float maxD = 1.0f)
        : x(x), y(y), width(w), height(h), minDepth(minD), maxDepth(maxD) {}
};

// 矩形区域
struct Rect {
    i32 x = 0;
    i32 y = 0;
    u32 width = 0;
    u32 height = 0;

    Rect() = default;
    Rect(i32 x, i32 y, u32 w, u32 h)
        : x(x), y(y), width(w), height(h) {}
};

// 偏移
struct Offset3D {
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;

    Offset3D() = default;
    Offset3D(i32 x, i32 y, i32 z)
        : x(x), y(y), z(z) {}
};

// 颜色
struct Color {
    float r;
    float g;
    float b;
    float a;

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

// 组件映射
struct ComponentMapping {
    enum class Swizzle {
        Identity,
        Zero,
        One,
        R,
        G,
        B,
        A
    };

    Swizzle r = Swizzle::Identity;
    Swizzle g = Swizzle::Identity;
    Swizzle b = Swizzle::Identity;
    Swizzle a = Swizzle::Identity;
};

// 子资源范围
struct SubresourceRange {
    u32 baseMipLevel = 0;
    u32 levelCount = 1;
    u32 baseArrayLayer = 0;
    u32 layerCount = 1;
};

// 子资源层
struct SubresourceLayers {
    u32 mipLevel = 0;
    u32 baseArrayLayer = 0;
    u32 layerCount = 1;
};

// 缓冲区拷贝区域
struct BufferCopy {
    u64 srcOffset = 0;
    u64 dstOffset = 0;
    u64 size = 0;
};

// 图像拷贝区域
struct ImageCopy {
    SubresourceLayers srcSubresource;
    Offset3D srcOffset;
    SubresourceLayers dstSubresource;
    Offset3D dstOffset;
    ResourceExtent extent;
};

// 缓冲区到图像拷贝
struct BufferImageCopy {
    u64 bufferOffset = 0;
    u32 bufferRowLength = 0;
    u32 bufferImageHeight = 0;
    SubresourceLayers imageSubresource;
    Offset3D imageOffset;
    ResourceExtent imageExtent;
};

/**
 * @brief 顶点输入率
 * @details 顶点属性的更新频率
 */
enum class VertexInputRate {
    PerVertex,      ///< 每个顶点更新一次
    PerInstance,    ///< 每个实例更新一次（实例化渲染）
    Count           ///< 输入率数量
};

// 顶点输入绑定
struct VertexInputBinding {
    u32 binding = 0;
    u32 stride = 0;
    VertexInputRate inputRate = VertexInputRate::PerVertex;
};

// 顶点输入属性
struct VertexInputAttribute {
    u32 location = 0;
    u32 binding = 0;
    Format format = Format::RGBA32_FLOAT;
    u32 offset = 0;
};

// 光栅化状态
struct RasterizerState {
    FillMode fillMode = FillMode::Solid;
    CullMode cullMode = CullMode::Back;
    bool frontCounterClockwise = false;
    i32 depthBias = 0;
    float depthBiasClamp = 0.0f;
    float slopeScaledDepthBias = 0.0f;
    bool depthClipEnable = true;
    bool scissorEnable = false;
    bool multisampleEnable = false;
    bool antialiasedLineEnable = false;
    u32 forcedSampleCount = 0;
    bool conservativeRaster = false;
};

// 深度模板状态
struct DepthStencilState {
    bool depthEnable = true;
    bool depthWriteEnable = true;
    ComparisonFunc depthFunc = ComparisonFunc::Less;
    bool stencilEnable = false;
    u8 stencilReadMask = 0xFF;
    u8 stencilWriteMask = 0xFF;

    struct StencilOpState {
        StencilOp failOp = StencilOp::Keep;
        StencilOp depthFailOp = StencilOp::Keep;
        StencilOp passOp = StencilOp::Keep;
        ComparisonFunc func = ComparisonFunc::Always;
    };

    StencilOpState frontFace;
    StencilOpState backFace;
};

// 渲染目标混合状态
struct RenderTargetBlendState {
    bool blendEnable = false;
    BlendFactor srcBlend = BlendFactor::One;
    BlendFactor destBlend = BlendFactor::Zero;
    BlendOp blendOp = BlendOp::Add;
    BlendFactor srcBlendAlpha = BlendFactor::One;
    BlendFactor destBlendAlpha = BlendFactor::Zero;
    BlendOp blendOpAlpha = BlendOp::Add;
    u8 renderTargetWriteMask = 0xF;
};

// 混合状态
struct BlendState {
    bool alphaToCoverageEnable = false;
    bool independentBlendEnable = false;
    std::array<RenderTargetBlendState, MaxRenderTargets> renderTarget;
};

// 采样器状态
struct SamplerState {
    Filter filter = Filter::Linear;
    AddressMode addressU = AddressMode::Wrap;
    AddressMode addressV = AddressMode::Wrap;
    AddressMode addressW = AddressMode::Wrap;
    float mipLODBias = 0.0f;
    u32 maxAnisotropy = 1;
    ComparisonFunc comparisonFunc = ComparisonFunc::Never;
    std::array<float, 4> borderColor = {0.0f, 0.0f, 0.0f, 1.0f};
    float minLOD = 0.0f;
    float maxLOD = 3.402823466e+38f; // FLT_MAX
};

// 深度边界
struct DepthBounds {
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
};

/**
 * @brief GPU设备特性
 * @details 可选的GPU功能支持标志
 */
enum class Feature : u32 {
    None = 0,
    // 基础特性
    GeometryShader = 1 << 0,        ///< 几何着色器支持
    Tessellation = 1 << 1,          ///< 曲面细分支持
    ComputeShader = 1 << 2,         ///< 计算着色器支持
    // 高级特性
    RayTracing = 1 << 3,            ///< 光线追踪支持
    MeshShader = 1 << 4,            ///< 网格着色器支持
    VariableRateShading = 1 << 5,   ///< 可变速率着色支持
    // 内存特性
    MemoryBudget = 1 << 6,          ///< 内存预算查询支持
    // 其他
    TimelineSemaphore = 1 << 7,     ///< 时间线信号量支持
    DescriptorIndexing = 1 << 8,    ///< 描述符索引支持（绑定-less）
    BufferDeviceAddress = 1 << 9,   ///< 缓冲区设备地址支持
    Count                           ///< 特性数量
};

// 设备信息
struct DeviceInfo {
    std::string deviceName;
    u32 vendorId = 0;
    u32 deviceId = 0;
    RenderAPI api = RenderAPI::Vulkan;
    u32 apiVersionMajor = 1;
    u32 apiVersionMinor = 0;
    u32 apiVersionPatch = 0;
    u64 localMemorySize = 0;
    u64 sharedMemorySize = 0;
    u32 maxTextureDimension1D = 0;
    u32 maxTextureDimension2D = 0;
    u32 maxTextureDimension3D = 0;
    u32 maxTextureArrayLayers = 0;
    u32 maxConstantBufferSize = 0;
    u32 maxPushConstantsSize = 0;
    u32 maxSamplerAnisotropy = 0;
    u32 maxBoundDescriptorSets = 0;
    std::vector<Feature> supportedFeatures;
};

/**
 * @brief GPU适配器类型
 * @details 区分不同类型的GPU硬件
 */
enum class AdapterType {
    Integrated, ///< 集成显卡（共享系统内存）
    Discrete,   ///< 独立显卡（专用显存）
    Virtual,    ///< 虚拟GPU
    CPU,        ///< 软件/CPU模拟
    Count       ///< 适配器类型数量
};

// 适配器信息
struct AdapterInfo {
    std::string name;
    u32 vendorId = 0;
    u32 deviceId = 0;
    AdapterType type = AdapterType::Discrete;
    u64 dedicatedMemory = 0;
    u64 sharedMemory = 0;
};

// 渲染系统配置
struct RenderSystemConfig {
    RenderAPI api = RenderAPI::Vulkan;
    AdapterType preferredAdapter = AdapterType::Discrete;
    void* windowHandle = nullptr;
    ResourceExtent windowExtent = {1280, 720};
    bool enableValidation = true;
    bool enableDebugMarkers = true;
    bool enableApiDump = false;
    u32 maxFramesInFlight = 3;
    u32 graphicsQueueCount = 1;
    u32 computeQueueCount = 0;
    u32 transferQueueCount = 0;
};

// 帧信息
struct FrameInfo {
    u64 frameNumber = 0;
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    u32 width = 0;
    u32 height = 0;
};

// 内存统计
struct MemoryStatistics {
    u64 totalAllocated = 0;
    u64 totalUsed = 0;
    u64 textureMemory = 0;
    u64 bufferMemory = 0;
    u64 renderTargetMemory = 0;
    u64 transientMemory = 0;
    u32 resourceCount = 0;
    u32 textureCount = 0;
    u32 bufferCount = 0;
};

/**
 * @brief 附件加载操作
 * @details 渲染通道开始时对附件内容的操作
 */
enum class LoadOp {
    Load,       ///< 保留现有内容（读取上一次结果）
    Clear,      ///< 清除为指定值（最常见，性能最优）
    DontCare    ///< 内容未定义（最高性能，内容不可预测）
};

/**
 * @brief 附件存储操作
 * @details 渲染通道结束时对附件内容的操作
 */
enum class StoreOp {
    Store,      ///< 写入内存（保留结果）
    DontCare    ///< 可能不写入（性能优化，结果可能丢失）
};

// 渲染通道附件描述（用于Subpass合并）
struct AttachmentDesc {
    Format format = Format::Unknown;
    u32 sampleCount = 1;
    ImageLayout initialLayout = ImageLayout::Undefined;
    ImageLayout finalLayout = ImageLayout::ShaderReadOnly;
    ImageLayout referenceLayout = ImageLayout::ColorAttachment;  // 子通道中的布局
    LoadOp loadOp = LoadOp::Clear;
    StoreOp storeOp = StoreOp::Store;
    ClearValue clearValue;
};

// 渲染通道描述（传统Vulkan RenderPass兼容）
struct RenderPassAttachmentDesc {
    std::vector<AttachmentDesc> attachments;
    std::vector<SubpassDesc> subpasses;
    std::vector<SubpassDependency> dependencies;
};

/**
 * @brief 分离屏障阶段
 * @details 用于分阶段屏障同步
 */
enum class SplitBarrierPhase {
    Begin,      ///< 开始阶段 - 释放资源访问权
    End         ///< 结束阶段 - 获取资源访问权
};

/**
 * @brief 渲染特性级别
 * @details 对应Vulkan/DirectX的不同功能集版本
 */
enum class FeatureLevel {
    Level_1_0,  ///< 基础特性集
    Level_1_1,  ///< 扩展特性集
    Level_1_2,  ///< 高级特性集
    Level_1_3,  ///< 最新特性集
    Count       ///< 特性级别数量
};

// 资源别名信息（移到ResourceHandle定义之后）

// ============================================================================
// 强类型句柄定义（从Handle.h合并，避免循环依赖）
// ============================================================================

// 资源标签（用于强类型句柄）
struct ResourceTag {};
struct TextureTag {};
struct BufferTag {};
struct SamplerTag {};
struct ResourceViewTag {};
struct PipelineTag {};
struct PipelineLayoutTag {};
struct ShaderTag {};
struct DescriptorSetTag {};
struct DescriptorSetLayoutTag {};
struct RenderPassTag {};
struct FramebufferTag {};
struct RenderGraphTag {};
struct RenderGraphPassTag {};
struct QueryPoolTag {};
struct FenceTag {};
struct SemaphoreTag {};
struct CommandPoolTag {};
struct CommandBufferTag {};
struct JobTag {};

// 强类型句柄模板
template<typename Tag>
class HandleType {
public:
    HandleType() : handle(InvalidHandle) {}
    explicit HandleType(Handle h) : handle(h) {}

    [[nodiscard]] bool isValid() const { return handle != InvalidHandle; }
    [[nodiscard]] Handle get() const { return handle; }
    void reset() { handle = InvalidHandle; }

    explicit operator bool() const { return isValid(); }
    operator Handle() const { return handle; }

    [[nodiscard]] bool operator==(const HandleType& other) const { return handle == other.handle; }
    [[nodiscard]] bool operator!=(const HandleType& other) const { return handle != other.handle; }
    [[nodiscard]] bool operator<(const HandleType& other) const { return handle < other.handle; }
    [[nodiscard]] bool operator>(const HandleType& other) const { return handle > other.handle; }

private:
    Handle handle;
};

// 强类型句柄定义
using ResourceHandle = HandleType<ResourceTag>;
using TextureHandle = HandleType<TextureTag>;
using BufferHandle = HandleType<BufferTag>;
using SamplerHandle = HandleType<SamplerTag>;
using ResourceViewHandle = HandleType<ResourceViewTag>;
using PipelineHandle = HandleType<PipelineTag>;
using PipelineLayoutHandle = HandleType<PipelineLayoutTag>;
using ShaderHandle = HandleType<ShaderTag>;
using DescriptorSetHandle = HandleType<DescriptorSetTag>;
using DescriptorSetLayoutHandle = HandleType<DescriptorSetLayoutTag>;
using RenderPassHandle = HandleType<RenderPassTag>;
using FramebufferHandle = HandleType<FramebufferTag>;
using RenderGraphHandle = HandleType<RenderGraphTag>;
using RenderGraphPassHandle = HandleType<RenderGraphPassTag>;
using QueryPoolHandle = HandleType<QueryPoolTag>;
using FenceHandle = HandleType<FenceTag>;
using SemaphoreHandle = HandleType<SemaphoreTag>;
using CommandPoolHandle = HandleType<CommandPoolTag>;
using CommandBufferHandle = HandleType<CommandBufferTag>;
using JobHandle = HandleType<JobTag>;

// 句柄分配器
template<typename HandleType>
class HandleAllocator {
public:
    HandleAllocator() : nextHandle(1) {}

    [[nodiscard]] HandleType allocate() {
        if (!freeHandles.empty()) {
            HandleType handle = freeHandles.back();
            freeHandles.pop_back();
            return handle;
        }
        return HandleType(nextHandle++);
    }

    void deallocate(HandleType handle) {
        if (handle.isValid()) {
            freeHandles.push_back(handle);
        }
    }

    void reset() {
        nextHandle = 1;
        freeHandles.clear();
    }

    [[nodiscard]] u32 getCount() const {
        return nextHandle - 1 - static_cast<u32>(freeHandles.size());
    }

private:
    Handle nextHandle;
    std::vector<HandleType> freeHandles;
};

// 版本化句柄（用于资源别名追踪）
template<typename Tag>
class VersionedHandle {
public:
    VersionedHandle() : index(InvalidHandle), version(0) {}
    VersionedHandle(Handle idx, u32 ver) : index(idx), version(ver) {}

    [[nodiscard]] bool isValid() const { return index != InvalidHandle; }
    [[nodiscard]] Handle getIndex() const { return index; }
    [[nodiscard]] u32 getVersion() const { return version; }

    [[nodiscard]] VersionedHandle nextVersion() const {
        return VersionedHandle(index, version + 1);
    }

    [[nodiscard]] bool operator==(const VersionedHandle& other) const {
        return index == other.index && version == other.version;
    }

    [[nodiscard]] bool operator!=(const VersionedHandle& other) const {
        return !(*this == other);
    }

    struct Hash {
        [[nodiscard]] size_t operator()(const VersionedHandle& h) const {
            return std::hash<Handle>{}(h.getIndex()) ^ (std::hash<u32>{}(h.getVersion()) << 1);
        }
    };

private:
    Handle index;
    u32 version;
};

// 版本化资源句柄
using VersionedResourceHandle = VersionedHandle<ResourceTag>;
using VersionedTextureHandle = VersionedHandle<TextureTag>;
using VersionedBufferHandle = VersionedHandle<BufferTag>;

// 资源别名信息（在ResourceHandle定义之后）
struct ResourceAliasingInfo {
    ResourceHandle beforeResource;
    ResourceHandle afterResource;
    bool discardBeforeContent = false;  // 是否丢弃之前资源的内容
};

} // namespace render