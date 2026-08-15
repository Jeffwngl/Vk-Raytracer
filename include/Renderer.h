#pragma once

#include "Core.h"
#include "ComputePipeline.h"
#include "ComputeDescriptorSet.h"
#include "Queue.h"

#include <iostream>
#include <stdbool.h>
#include <stdint.h>

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool initialize(VulkanCore& vkCore);

    void drawFrame(FrameData& frame);
    void cleanUp();

private:
    void waitForFences(FrameData& frame);
    bool acquireSwapchainImage(FrameData& frame, uint32_t& imageIndex);
    void resetFences(FrameData& frame);
    void resetCommandBuffers(FrameData& frame);
    void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void submitCommandBuffers();
    void presentSwapchainImage();

    void createOutputImage();
    void createOutputImageView();
    // https://docs.vulkan.org/guide/latest/storage_image_and_texel_buffers.html
    // transition outputImage from undefined to general
    void transitionImage(
		VkCommandBuffer commandBuffer,
		VKImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);

private:
    VulkanCore* vulkanCore{ nullptr };
    uint32_t currentFrame{ 0 };

    ComputePipeline computePipeline;
    ComputeDescriptorSet computeDescriptorSet;
    VkImage outputImage{ VK_NULL_HANDLE };
    VkImageView outputImageView{ VK_NULL_HANDLE };
    VmaAllocation outputImageAllocation{ VK_NULL_HANDLE };
	// VkQueue queue{ VK_NULL_HANDLE };
	Queue queue{ nullptr };
	// TODO: swith to queue class
};
