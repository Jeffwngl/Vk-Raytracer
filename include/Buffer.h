#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Vulkan {

class Buffer {
public:
    Buffer() = default;
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void initialize(
        VmaAllocator allocator,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags allocationFlags = 0
    );

    void upload(
        const void* data,
        VkDeviceSize size
    );

    VkBuffer get() const {
        return buffer;
    }

    VkDeviceSize getSize() const {
        return size;
    }

    void cleanUp();

private:
    VmaAllocator allocator{ VK_NULL_HANDLE };

    VkBuffer buffer{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };

    VkDeviceSize size{ 0 };

    void* mappedData{ nullptr };
};

}