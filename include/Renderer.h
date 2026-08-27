#pragma once

#include "Core.h"
#include "ComputePipeline.h"
#include "ComputeDescriptorSet.h"
#include "Buffer.h"
#include "Scene.h"

#include <stdbool.h>
#include <stdint.h>
#include <string>

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool initialize(Vulkan::VulkanCore& vkCore, const Scene& scene);

    void drawFrame();
    void cleanUp();

private:
    void waitForFences(VkDevice device, Vulkan::FrameData& frame);
    bool acquireSwapchainImage(VkDevice device, VkSwapchainKHR swapchain, Vulkan::FrameData& frame, uint32_t& imageIndex);
    void resetFences(VkDevice device, Vulkan::FrameData& frame);
    void resetCommandBuffers(Vulkan::FrameData& frame);
    void recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createOutputImage();
    void createOutputImageView();
    void createSceneBuffer();
    void createComputeDescriptorSet();
    void createComputePipeline(std::string& path);

    // https://docs.vulkan.org/guide/latest/storage_image_and_texel_buffers.html
    // transition outputImage from undefined to general
    void transitionImage(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
    uint32_t currentFrame{ 0 };
    bool outputImageInitialized{ false };

    Vulkan::ComputePipeline computePipeline{};
    Vulkan::ComputeDescriptorSet computeDescriptorSet{};
    Vulkan::Buffer sceneObjectBuffer{};
    const Scene* scene{ nullptr };
    VkImage outputImage{ VK_NULL_HANDLE };
    VkImageView outputImageView{ VK_NULL_HANDLE };
    VmaAllocation outputImageAllocation{ VK_NULL_HANDLE };
    uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };
};
