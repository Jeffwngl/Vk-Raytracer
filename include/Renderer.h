#pragma once

#include "Core.h"
#include "ComputePipeline.h"
#include "ComputeDescriptorSet.h"
#include "Queue.h"

#include <stdbool.h>
#include <stdint.h>

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool initialize(VulkanCore& vkCore);

    void drawFrame();
    void cleanUp();

private:
    void waitForFences(VkDevice device, FrameData& frame);
    bool acquireSwapchainImage(VkDevice device, VkSwapchainKHR swapchain, FrameData& frame, uint32_t& imageIndex);
    void resetFences(VkDevice device, FrameData& frame);
    void resetCommandBuffers(FrameData& frame);
    void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createOutputImage();
    void createOutputImageView();
    // https://docs.vulkan.org/guide/latest/storage_image_and_texel_buffers.html
    // transition outputImage from undefined to general
    void transitionImage(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);

private:
    VulkanCore* vulkanCore{ nullptr };
    uint32_t currentFrame{ 0 };
    bool outputImageInitialized{ false };

    ComputePipeline computePipeline;
    ComputeDescriptorSet computeDescriptorSet;
    VkImage outputImage{ VK_NULL_HANDLE };
    VkImageView outputImageView{ VK_NULL_HANDLE };
    VmaAllocation outputImageAllocation{ VK_NULL_HANDLE };
    uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };
};
