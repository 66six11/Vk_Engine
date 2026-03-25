#include "../../include/managers/BufferManager.h"
#include <cassert>

namespace engine::rhi {

BufferManager::BufferManager(VkDevice device, VmaAllocator allocator)
    : m_device(device), m_allocator(allocator) {
    assert(device != VK_NULL_HANDLE);
    assert(allocator != VK_NULL_HANDLE);
}

BufferManager::~BufferManager() {
    for (auto it = begin(); it != end(); ++it) {
        Buffer& buffer = *it;
        if (buffer.buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
        }
    }
    clear();
}

BufferHandle BufferManager::createBuffer(const BufferDesc& desc) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = desc.usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = desc.memoryUsage;
    
    if (desc.mapped) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    Buffer buffer = {};
    VkResult result = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo,
                                      &buffer.buffer, &buffer.allocation, &buffer.allocInfo);

    if (result != VK_SUCCESS) {
        return BufferHandle();
    }

    buffer.size = desc.size;
    buffer.usage = desc.usage;

    return create(std::move(buffer));
}

VkDeviceAddress BufferManager::getBufferAddress(BufferHandle handle) const {
    const Buffer* buffer = get(handle);
    if (!buffer || buffer->buffer == VK_NULL_HANDLE) {
        return 0;
    }

    VkBufferDeviceAddressInfo addrInfo = {};
    addrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addrInfo.buffer = buffer->buffer;

    return vkGetBufferDeviceAddress(m_device, &addrInfo);
}

} // namespace engine::rhi