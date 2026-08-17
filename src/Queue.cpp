#pragma once

#include "Queue.h"
#include "Utils.h"

VkQueue Queue::get() const {
    return queue;
}

void Queue::initialize(
        VkDevice device,
        uint32_t queueFamily, 
        uint32_t queueIndex
    ) {

	vkGetDeviceQueue(
		device,
		queueFamily,
		queueIndex,
		&queue
	);
	
	std::cout << "Queue acquired" << '\n';
}

void Queue::waitIdle() const {
	vkQueueWaitIdle(queue);
}

void Queue::submit(
    VkCommandBuffer commandBuffer,
    VkSemaphore waitSemaphore,
    VkPipelineStageFlags waitStage,
    VkSemaphore signalSemaphore,
    VkFence fence
) const {
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,
        .pWaitDstStageMask = &waitStage,

        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signalSemaphore
    };

    utils::check(vkQueueSubmit(
        queue,
        1,
        &submitInfo,
        fence
    ));
}

VkResult Queue::present(
    VkSwapchainKHR swapchain,
    uint32_t imageIndex,
    VkSemaphore waitSemaphore
) const {
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,

        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex
    };

    return vkQueuePresentKHR(
        queue,
        &presentInfo
    );
}
