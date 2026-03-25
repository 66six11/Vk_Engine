#pragma once
#ifndef ENGINE_RHI_RESOURCE_MANAGER_H
#define ENGINE_RHI_RESOURCE_MANAGER_H

#include "SlotMap.h"
#include <utility>

namespace engine::rhi {

template<typename T, typename HandleType>
class ResourceManager {
public:
    using ResourceType = T;

    ResourceManager() = default;
    virtual ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    ResourceManager(ResourceManager&&) noexcept = default;
    ResourceManager& operator=(ResourceManager&&) noexcept = default;

    template<typename... Args>
    [[nodiscard]] HandleType create(Args&&... args) {
        Handle handle = m_storage.allocate();
        if (T* slot = m_storage.get(handle)) {
            ::new (static_cast<void*>(slot)) T(std::forward<Args>(args)...);
        }
        return HandleType(handle);
    }

    void destroy(HandleType handle) {
        if (T* ptr = get(handle)) {
            ptr->~T();
            m_storage.free(handle.raw());
        }
    }

    [[nodiscard]] T* get(HandleType handle) {
        return m_storage.get(handle.raw());
    }

    [[nodiscard]] const T* get(HandleType handle) const {
        return m_storage.get(handle.raw());
    }

    [[nodiscard]] bool isValid(HandleType handle) const {
        return m_storage.isValid(handle.raw());
    }

    [[nodiscard]] size_t size() const noexcept { return m_storage.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_storage.empty(); }

    void clear() { m_storage.clear(); }

    [[nodiscard]] auto begin() { return m_storage.begin(); }
    [[nodiscard]] auto end() { return m_storage.end(); }
    [[nodiscard]] auto begin() const { return m_storage.begin(); }
    [[nodiscard]] auto end() const { return m_storage.end(); }

protected:
    SlotMap<T> m_storage;
};

template<typename T, typename HandleType, typename ManagerType>
class ResourceRef {
public:
    ResourceRef() noexcept = default;
    ResourceRef(HandleType handle, ManagerType* manager) noexcept
        : m_handle(handle), m_manager(manager) {}

    ResourceRef(const ResourceRef&) = delete;
    ResourceRef& operator=(const ResourceRef&) = delete;

    ResourceRef(ResourceRef&& other) noexcept
        : m_handle(other.m_handle), m_manager(other.m_manager) {
        other.m_handle = HandleType();
        other.m_manager = nullptr;
    }

    ResourceRef& operator=(ResourceRef&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = other.m_handle;
            m_manager = other.m_manager;
            other.m_handle = HandleType();
            other.m_manager = nullptr;
        }
        return *this;
    }

    ~ResourceRef() { reset(); }

    void reset() {
        if (m_manager && m_handle.isValid()) {
            m_manager->destroy(m_handle);
        }
        m_handle = HandleType();
        m_manager = nullptr;
    }

    [[nodiscard]] T* get() const {
        return m_manager ? m_manager->get(m_handle) : nullptr;
    }

    [[nodiscard]] T& operator*() const {
        T* ptr = get();
        assert(ptr != nullptr);
        return *ptr;
    }

    [[nodiscard]] T* operator->() const {
        T* ptr = get();
        assert(ptr != nullptr);
        return ptr;
    }

    [[nodiscard]] HandleType handle() const noexcept { return m_handle; }
    [[nodiscard]] bool isValid() const noexcept {
        return m_manager != nullptr && m_manager->isValid(m_handle);
    }

    explicit operator bool() const noexcept { return isValid(); }

private:
    HandleType m_handle;
    ManagerType* m_manager = nullptr;
};

} // namespace engine::rhi

#endif // ENGINE_RHI_RESOURCE_MANAGER_H