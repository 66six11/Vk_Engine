#pragma once
#ifndef ENGINE_RHI_TYPES_H
#define ENGINE_RHI_TYPES_H

// RHI 类型统一入口文件
// 包含所有 Handle 和 Tag 定义

#include <core/Handle.h>
#include <core/TypedHandle.h>

namespace engine::rhi {

//=============================================================================
// 资源类型 Tag 声明（RHI 专用）
//=============================================================================

struct PipelineLayoutTag {};
struct GraphicsPipelineTag {};
struct ComputePipelineTag {};

//=============================================================================
// 类型安全的 Handle 别名（RHI 专用）
//=============================================================================

using PipelineLayoutHandle = TypedHandle<PipelineLayoutTag>;
using GraphicsPipelineHandle = TypedHandle<GraphicsPipelineTag>;
using ComputePipelineHandle = TypedHandle<ComputePipelineTag>;

} // namespace engine::rhi

//=============================================================================
// std::hash 特化（RHI 专用 Handle）
//=============================================================================

namespace std {

template<>
struct hash<engine::rhi::PipelineLayoutHandle> {
    [[nodiscard]] size_t operator()(const engine::rhi::PipelineLayoutHandle& h) const noexcept {
        return std::hash<uint64_t>{}(h.value());
    }
};

template<>
struct hash<engine::rhi::GraphicsPipelineHandle> {
    [[nodiscard]] size_t operator()(const engine::rhi::GraphicsPipelineHandle& h) const noexcept {
        return std::hash<uint64_t>{}(h.value());
    }
};

template<>
struct hash<engine::rhi::ComputePipelineHandle> {
    [[nodiscard]] size_t operator()(const engine::rhi::ComputePipelineHandle& h) const noexcept {
        return std::hash<uint64_t>{}(h.value());
    }
};

} // namespace std

#endif // ENGINE_RHI_TYPES_H