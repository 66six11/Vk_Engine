#pragma once
#ifndef ENGINE_RHI_FWD_H
#define ENGINE_RHI_FWD_H

// 前向声明文件，用于解决循环依赖

#include <core/TypedHandle.h>

// 资源类型前向声明
namespace engine::rhi {

struct Buffer;
struct Texture;
struct Shader;
struct Sampler;
struct GraphicsPipeline;
struct ComputePipeline;
// PipelineLayout 定义在 Shader.h 中

} // namespace engine::rhi

// 确保 TypedHandle 定义可用
#include <rhi/Buffer.h>
#include <rhi/Texture.h>
#include <rhi/Shader.h>
#include <rhi/Sampler.h>

#endif // ENGINE_RHI_FWD_H
