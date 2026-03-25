//
// Created by C66 on 2026/3/23.
//
#pragma once
#ifndef ENGINE_TYPE_H
#define ENGINE_TYPE_H

//图像格式
// 参考 Unity TextureFormat 分类命名风格
// 包含颜色 (Color)、深度/模板 (Depth/Stencil) 及压缩格式
enum class Format {
    // ==================== 未定义 ====================
    Unknown,

    // ==================== 颜色格式：8-bit 每通道 ====================
    // 标准 RGBA8 (sRGB/Linear)
    RGBA8_UNORM,        // 对应 Unity: RGBA32
    RGBA8_SRGB,         // 对应 Unity: RGBA32 (sRGB)
    
    // 标准 BGRA8 (常用于屏幕输出)
    BGRA8_UNORM,        // 对应 Unity: BGRA32
    BGRA8_SRGB,

    // RGB8 (无 Alpha)
    RGB8_UNORM,         // 对应 Unity: RGB24
    RGB8_SRGB,          // 对应 Unity: RGB24 (sRGB)

    // 单通道/双通道 (掩码、高度图、法线 XY)
    R8_UNORM,           // 对应 Unity: R8
    RG8_UNORM,          // 对应 Unity: RG16 (注意 Unity 命名有时混用位数，此处指 8bit*2)
    R8_SNORM,
    RG8_SNORM,

    // ==================== 颜色格式：16-bit 每通道 (HDR/高精度) ====================
    // 半精度浮点 (Half Float)
    RGBA16_FLOAT,       // 对应 Unity: RGBAHalf
    RGB16_FLOAT,        // 对应 Unity: RGBHalf
    R16_FLOAT,          // 对应 Unity: RHalf
    
    // 16-bit 整型/归一化
    RGBA16_UNORM,       // 对应 Unity: RGBA16
    R16_UNORM,

    // ==================== 颜色格式：32-bit 每通道 (超高精度) ====================
    // 全精度浮点
    RGBA32_FLOAT,       // 对应 Unity: RGBAFloat
    RGB32_FLOAT,        // 对应 Unity: RGBFloat
    R32_FLOAT,          // 对应 Unity: RFloat

    // ==================== 压缩格式 (BC/DXT/ETC) ====================
    // DXT/BC1 (4bpp)
    BC1_RGB_UNORM,      // 对应 Unity: DXT1 / RGBCrunch
    BC1_RGBA_UNORM,     // 对应 Unity: DXT1Crunched (with alpha)
    BC1_RGB_SRGB,
    BC1_RGBA_SRGB,

    // DXT/BC2 (8bpp)
    BC2_UNORM,          // 对应 Unity: DXT5
    BC2_SRGB,

    // DXT/BC3 (8bpp, 更好的 Alpha 插值)
    BC3_UNORM,          // 对应 Unity: DXT5Crunched
    BC3_SRGB,

    // 单通道/双通道压缩 (4bpp/8bpp)
    BC4_UNORM,          // 对应 Unity: BC4 (R 压缩)
    BC4_SNORM,
    BC5_UNORM,          // 对应 Unity: BC5 (RG 压缩，常用于法线)
    BC5_SNORM,

    // HDR 压缩
    BC6H_UF16,          // 对应 Unity: BC6H
    BC6H_SF16,

    // 高质量 RGBA 压缩
    BC7_UNORM,          // 对应 Unity: BC7
    BC7_SRGB,

    // ==================== 深度与模板格式 (Depth & Stencil) ====================
    // 纯深度格式
    Depth16_UNORM,      // 对应 Unity: Depth16
    Depth24_UNORM,      // 对应 Unity: Depth24 (通常配合 Stencil 使用，但单独存在)
    Depth32_FLOAT,      // 对应 Unity: Depth32

    // 深度 + 模板组合格式
    Depth24_Stencil8,   // 对应 Unity: Depth24Stencil8 (最常用)
    Depth32_Float_Stencil8, // 对应 Unity: Depth32Stencil8

    // 纯模板格式
    Stencil8            // 对应 Unity: Stencil8
};




#endif //ENGINE_TYPE_H