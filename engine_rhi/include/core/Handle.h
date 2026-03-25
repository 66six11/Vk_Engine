#pragma once
#ifndef ENGINE_RHI_HANDLE_H
#define ENGINE_RHI_HANDLE_H

#include <cstdint>
#include <functional>

namespace engine::rhi {

// 64-bit Handle: [32-bit generation | 32-bit index]
// 支持 40亿个索引，每个索引可重用 40亿次
class Handle {
public:
    using IndexType = uint32_t;
    using GenerationType = uint32_t;

    constexpr Handle() noexcept : m_value(0) {}
    constexpr Handle(IndexType index, GenerationType generation) noexcept
        : m_value((static_cast<uint64_t>(generation) << 32) | index) {}

    [[nodiscard]] constexpr IndexType index() const noexcept {
        return static_cast<IndexType>(m_value);
    }

    [[nodiscard]] constexpr GenerationType generation() const noexcept {
        return static_cast<GenerationType>(m_value >> 32);
    }

    [[nodiscard]] constexpr bool isValid() const noexcept { return m_value != 0; }
    [[nodiscard]] constexpr uint64_t value() const noexcept { return m_value; }

    constexpr bool operator==(const Handle& other) const noexcept = default;
    constexpr bool operator!=(const Handle& other) const noexcept = default;

    // 提供从 Handle 到 bool 的隐式转换（仅用于条件判断）
    constexpr explicit operator bool() const noexcept { return isValid(); }

private:
    uint64_t m_value;
};

} // namespace engine::rhi

// std::hash 特化
namespace std {

template<>
struct hash<engine::rhi::Handle> {
    [[nodiscard]] size_t operator()(const engine::rhi::Handle& h) const noexcept {
        return std::hash<uint64_t>{}(h.value());
    }
};

} // namespace std

#endif // ENGINE_RHI_HANDLE_H
