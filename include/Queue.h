#include <vulkan/vulkan.h>

class Queue {
public:
	Queue() = default;
	~Queue();
	
	void initialize(VkDevice device, VkSwapchainKHR swapchain, uint32_t queueFamily, uint32_t queueIndex);
	void waitIdle();
	uint32_t acquireNextImage();
	void submitSync(VkCommandBuffer commandBuffer);
	void submitAsync(VkCommandBuffer commandBuffer);
	void present(uint32_t imageIndex);
private:

private:
	VkDevice device{ VK_NULL_HANDLE };
	VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
	VkQueue queue{ VK_NULL_HANDLE };
	VkSemaphore renderComplete;
	VkSemaphore presentComplete;
};
