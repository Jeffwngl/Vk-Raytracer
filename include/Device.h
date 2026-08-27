#pragma once

#include <vulkan/vulkan.h>

#include "Queue.h"

namespace Vulkan {

class Device {
public:
    bool initialize(
        VkInstance instance,
        VkSurfaceKHR surface
    );

    VkPhysicalDevice getPhysicalDevice() const;
    VkDevice get() const;

    uint32_t getQueueFamily() const;
    const Queue& getQueue() const;

private:
    void selectPhysicalDevice(
        VkInstance instance
    );
    bool checkSuitableDevice(
        VkPhysicalDevice device
    );
    void createLogicalDevice(
        VkInstance instance
    );

private:
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };

    uint32_t queueFamily{ 0 };
    Queue queue;
};

}