#pragma once
#ifndef ENGINE_RHI_SLOTMAP_H
#define ENGINE_RHI_SLOTMAP_H

#include "../rhi/RHITypes.h"
#include <vector>
#include <cassert>
#include <iterator>

namespace engine::rhi {

template<typename T>
class SlotMap {
public:
    static constexpr Handle::IndexType kInvalidIndex = ~0u;

    struct Slot {
        union {
            T data;
            Handle::IndexType nextFree;
        };
        Handle::GenerationType generation = 0;
        bool occupied = false;

        Slot() : nextFree(kInvalidIndex) {}
        ~Slot() {
            if (occupied) {
                data.~T();
            }
        }

        // 移动构造函数
        Slot(Slot&& other) noexcept
            : nextFree(other.nextFree)
            , generation(other.generation)
            , occupied(other.occupied) {
            if (occupied) {
                new (&data) T(std::move(other.data));
            }
        }

        // 移动赋值
        Slot& operator=(Slot&& other) noexcept {
            if (this != &other) {
                if (occupied) {
                    data.~T();
                }
                nextFree = other.nextFree;
                generation = other.generation;
                occupied = other.occupied;
                if (occupied) {
                    new (&data) T(std::move(other.data));
                }
            }
            return *this;
        }

        // 禁用拷贝
        Slot(const Slot&) = delete;
        Slot& operator=(const Slot&) = delete;
    };

    template<bool IsConst>
    class IteratorImpl {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;

        IteratorImpl() = default;
        IteratorImpl(Slot* slots, Handle::IndexType index, Handle::IndexType size)
            : m_slots(slots), m_index(index), m_size(size) {
            skipFreeSlots();
        }

        reference operator*() const {
            assert(m_index < m_size && m_slots[m_index].occupied);
            return m_slots[m_index].data;
        }

        pointer operator->() const {
            assert(m_index < m_size && m_slots[m_index].occupied);
            return &m_slots[m_index].data;
        }

        IteratorImpl& operator++() {
            ++m_index;
            skipFreeSlots();
            return *this;
        }

        IteratorImpl operator++(int) {
            IteratorImpl temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const IteratorImpl& other) const noexcept { return m_index == other.m_index; }
        bool operator!=(const IteratorImpl& other) const noexcept { return !(*this == other); }
        Handle::IndexType index() const { return m_index; }

    private:
        void skipFreeSlots() {
            while (m_index < m_size && !m_slots[m_index].occupied) {
                ++m_index;
            }
        }

        Slot* m_slots = nullptr;
        Handle::IndexType m_index = 0;
        Handle::IndexType m_size = 0;
    };

    using Iterator = IteratorImpl<false>;
    using ConstIterator = IteratorImpl<true>;

    SlotMap() = default;
    SlotMap(const SlotMap&) = delete;
    SlotMap& operator=(const SlotMap&) = delete;

    SlotMap(SlotMap&& other) noexcept
        : m_slots(std::move(other.m_slots))
        , m_freeList(other.m_freeList)
        , m_size(other.m_size) {
        other.m_freeList = kInvalidIndex;
        other.m_size = 0;
    }

    SlotMap& operator=(SlotMap&& other) noexcept {
        if (this != &other) {
            clear();
            m_slots = std::move(other.m_slots);
            m_freeList = other.m_freeList;
            m_size = other.m_size;
            other.m_freeList = kInvalidIndex;
            other.m_size = 0;
        }
        return *this;
    }

    ~SlotMap() { clear(); }

    [[nodiscard]] Handle allocate() {
        Handle::IndexType index;

        if (m_freeList != kInvalidIndex) {
            index = m_freeList;
            m_freeList = m_slots[index].nextFree;
        } else {
            index = static_cast<Handle::IndexType>(m_slots.size());
            m_slots.emplace_back();
        }

        Slot& slot = m_slots[index];
        slot.occupied = true;
        ++slot.generation;
        ++m_size;

        return Handle(index, slot.generation);
    }

    void free(Handle handle) {
        if (!isValid(handle)) return;

        Slot& slot = m_slots[handle.index()];
        slot.data.~T();
        slot.occupied = false;
        slot.nextFree = m_freeList;
        m_freeList = handle.index();
        --m_size;
    }

    [[nodiscard]] T* get(Handle handle) {
        if (!isValid(handle)) return nullptr;
        return &m_slots[handle.index()].data;
    }

    [[nodiscard]] const T* get(Handle handle) const {
        if (!isValid(handle)) return nullptr;
        return &m_slots[handle.index()].data;
    }

    [[nodiscard]] bool isValid(Handle handle) const {
        const Handle::IndexType index = handle.index();
        if (index >= m_slots.size()) return false;
        const Slot& slot = m_slots[index];
        return slot.occupied && slot.generation == handle.generation();
    }

    [[nodiscard]] size_t size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] size_t capacity() const noexcept { return m_slots.size(); }

    void clear() {
        for (Handle::IndexType i = 0; i < m_slots.size(); ++i) {
            if (m_slots[i].occupied) {
                m_slots[i].data.~T();
            }
        }
        m_slots.clear();
        m_freeList = kInvalidIndex;
        m_size = 0;
    }

    [[nodiscard]] Iterator begin() {
        return Iterator(m_slots.data(), 0, static_cast<Handle::IndexType>(m_slots.size()));
    }

    [[nodiscard]] Iterator end() {
        return Iterator(m_slots.data(), static_cast<Handle::IndexType>(m_slots.size()),
                       static_cast<Handle::IndexType>(m_slots.size()));
    }

    [[nodiscard]] ConstIterator begin() const {
        return ConstIterator(m_slots.data(), 0, static_cast<Handle::IndexType>(m_slots.size()));
    }

    [[nodiscard]] ConstIterator end() const {
        return ConstIterator(m_slots.data(), static_cast<Handle::IndexType>(m_slots.size()),
                            static_cast<Handle::IndexType>(m_slots.size()));
    }

private:
    std::vector<Slot> m_slots;
    Handle::IndexType m_freeList = kInvalidIndex;
    size_t m_size = 0;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_SLOTMAP_H