#include "Queue.h"
#include "Core.h"
#include "Utils.h"

void Queue::initialize(VulkanCore vkCore, uint32_t queueFamily, uint32_t queueIndex) {
	// change to pass in vulkancore class only
	device = vkCore.getDevice();
	swapchain = vkCore.getSwapchain();
	vkGetDeviceQueue(
		device,
		queueFamily,
		queueIndex,
		&queue
	);
	
	std::cout << "Queue acquired" << '\n';
	
	renderComplete = device.createSemaphore();
	presentComplete = device.createSemaphore();			

	
}

void Queue::waitIdle() {
	vkQueueWaitIdle(queue);
}

uint32_t Queue::acquireNextImage() {
	uint32_t imageIndex{ 0 };
	utils::check(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, presentComplete, nullptr, &imageIndex);
	return imageIndex;
}

void Queue::submitAsync(VkCommandBuffer commandBuffer) {
	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	
	VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &presentComplete,
		.pWaitDstStageMask = &waitFlags,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderComplete
	};

	utils::check(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
}

void Queue::present(uint32_t imageIndex) {
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderComplete,
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex
	};

	utils::check(vkQueuePresentKHR(queue, &presentInfo));
};
