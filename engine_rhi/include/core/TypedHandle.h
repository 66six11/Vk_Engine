#pragma once
#ifndef ENGINE_RHI_TYPED_HANDLE_H
#define ENGINE_RHI_TYPED_HANDLE_H

#include "Handle.h"
#include <functional>

namespace engine::rhi {

// 类型安全的 Handle，编译期防止混用不同资源类型的 Handle
template<typename Tag>
class TypedHandle {
public:
    using IndexType = Handle::IndexType;
    using GenerationType = Handle::GenerationType;

    constexpr TypedHandle() noexcept = default;
    constexpr explicit TypedHandle(Handle handle) noexcept : m_handle(handle) {}
    constexpr TypedHandle(IndexType index, GenerationType generation) noexcept
        : m_handle(index, generation) {}

    [[nodiscard]] constexpr IndexType index() const noexcept { return m_handle.index(); }
    [[nodiscard]] constexpr GenerationType generation() const noexcept { return m_handle.generation(); }
    [[nodiscard]] constexpr bool isValid() const noexcept { return m_handle.isValid(); }
    [[nodiscard]] constexpr Handle raw() const noexcept { return m_handle; }
    [[nodiscard]] constexpr uint64_t value() const noexcept { return m_handle.value(); }

    constexpr bool operator==(const TypedHandle& other) const noexcept = default;
    constexpr bool operator!=(const TypedHandle& other) const noexcept = default;

    // 显式转换为 bool
    constexpr explicit operator bool() const noexcept { return isValid(); }

private:
    Handle m_handle;
};

// 资源类型的 Tag 定义
struct BufferTag {};
struct TextureTag {};
struct ShaderTag {};
struct PipelineTag {};
struct SamplerTag {};
struct FramebufferTag {};
struct CommandBufferTag {};

// 类型安全的 Handle 别名
using BufferHandle = TypedHandle<BufferTag>;
using TextureHandle = TypedHandle<TextureTag>;
using ShaderHandle = TypedHandle<ShaderTag>;
using PipelineHandle = TypedHandle<PipelineTag>;
using SamplerHandle = TypedHandle<SamplerTag>;
using FramebufferHandle = TypedHandle<FramebufferTag>;
using CommandBufferHandle = TypedHandle<CommandBufferTag>;

    
} // namespace engine::rhi

// std::hash 特化
namespace std {

template<typename Tag>
struct hash<engine::rhi::TypedHandle<Tag>> {
    [[nodiscard]] size_t operator()(const engine::rhi::TypedHandle<Tag>& h) const noexcept {
        return std::hash<uint64_t>{}(h.value());
    }
};

} // namespace std

#endif // ENGINE_RHI_TYPED_HANDLE_H
