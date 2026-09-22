#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan {

class Queue {
public:
    Queue() = default;
    ~Queue() = default;

    void initialize(
        VkDevice device,
        uint32_t queueFamily,
        uint32_t queueIndex
    );

    void waitIdle() const;

    VkQueue get() const;

    void submit(
        VkCommandBuffer commandBuffer,
        VkSemaphore waitSemaphore,
        VkPipelineStageFlags waitStage,
        VkSemaphore signalSemaphore,
        VkFence
    ) const;

    VkResult present(
        VkSwapchainKHR swapchain,
        uint32_t imageIndex,
        VkSemaphore waitSemaphore
    ) const;

private:
	VkQueue queue{ VK_NULL_HANDLE };
};

}