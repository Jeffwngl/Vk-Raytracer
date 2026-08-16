#pragma once

#include <vulkan/vulkan.h>

class Queue {
public:
    Queue() = default;
    ~Queue() = default;

    void initialize(
        VkDevice device,
        uint32_t queueFamily,
        uint32_t queueIndex
    );

    void waitIdle();

    VkQueue get() const;
	// uint32_t acquireNextImage();
	// void submitSync(VkCommandBuffer commandBuffer);
	// void submitAsync(VkCommandBuffer commandBuffer);
	// void present(uint32_t imageIndex);

private:
	// VkDevice device{ VK_NULL_HANDLE };
	// VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
	VkQueue queue{ VK_NULL_HANDLE };
	// VkSemaphore renderComplete;
	// VkSemaphore presentComplete;
};
