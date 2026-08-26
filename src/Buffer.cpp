#include "Buffer.h"
#include "Utils.h"

namespace Vulkan {

void Buffer::initialize(
    VmaAllocator allocator,
    VkDeviceSize bufferSize,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags allocationFlags
) {
    this->allocator = allocator;
    this->size = bufferSize;

    VkBufferCreateInfo bufferCI{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocationCI{
        .flags = allocationFlags,
        .usage = memoryUsage
    };

    VmaAllocationInfo allocationInfo{};

    utils::check(vmaCreateBuffer(
        allocator,
        &bufferCI,
        &allocationCI,
        &buffer,
        &allocation,
        &allocationInfo
    ));

    mappedData = allocationInfo.pMappedData;
}

void Buffer::upload(
    const void* data,
    VkDeviceSize uploadSize
) {
    if (mappedData == nullptr) {
        throw std::runtime_error(
            "Attempted to upload to unmapped buffer"
        );
    }

    if (uploadSize > size) {
        throw std::runtime_error(
            "Buffer upload exceeds buffer size"
        );
    }

    std::memcpy(
        mappedData,
        data,
        uploadSize
    );
}

void Buffer::cleanUp() {
    if (buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(
            allocator,
            buffer,
            allocation
        );

        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        mappedData = nullptr;
        size = 0;
    }
}

Buffer::~Buffer() {
    cleanUp();
}

}