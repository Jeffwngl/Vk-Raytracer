#pragma once

#include "Core.h"
#include "ComputePipeline.h"
#include "ComputeDescriptorSet.h"
#include "GraphicsPipeline.h"
#include "GraphicsDescriptorSet.h"
#include "Buffer.h"
#include "Scene.h"
#include "ImGuiLayer.h"
#include "RenderSettings.h"
#include "BVH.h"

#include <stdbool.h>
#include <stdint.h>
#include <string>

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    bool initialize(Vulkan::VulkanCore& vkCore, const Scene& scene);

    void drawFrame(ImGuiLayer& imgui, RenderSettings& settings, DebugSettings& debug);

    uint32_t getAccumulatedFrames() const;
    void advanceAccumulatedFrames();
    void resetAccumulatedFrames();

    void cleanUp();

private:
    void waitForFences(VkDevice device, Vulkan::FrameData& frame);
    bool acquireSwapchainImage(
        VkDevice device, 
        VkSwapchainKHR swapchain, 
        Vulkan::FrameData& frame, 
        uint32_t& imageIndex
    );
    void resetFences(VkDevice device, Vulkan::FrameData& frame);
    void resetCommandBuffers(Vulkan::FrameData& frame);
    void recordCommandBuffers(
        VkCommandBuffer commandBuffer, 
        uint32_t imageIndex, 
        ImGuiLayer& imgui,
        RenderSettings& settings,
        DebugSettings& debug
    );
    void recordRaytraceCommands(
        VkCommandBuffer commandBuffer,
        uint32_t imageIndex,
        RenderSettings& settings,
        DebugSettings& debug,
        uint32_t width,
        uint32_t height,
        ImGuiLayer& imgui
    );
    void recordDebugCommands(
        VkCommandBuffer commandBuffer,
        uint32_t imageIndex,
        RenderSettings& settings,
        DebugSettings& debug,
        uint32_t width,
        uint32_t height,
        ImGuiLayer& imgui
    );

    void createImages();
    void createImageViews();
    void createOutputImage();
    void createOutputImageView();
    void createAccumulatedImage();
    void createAccumulatedImageView();
    void createDepthImage();
    void createDepthImageView();
    void createBuffers();
    template<typename T>
    void createStorageBuffer(
        Vulkan::Buffer& buffer,
        const std::vector<T>& data
    );
    void createComputeDescriptorSet();
    void createComputePipeline(std::string& path);
    void createGraphicsDescriptorSet();
    void createGraphicsPipelineLayout();
    void createGraphicsPipeline(std::string& vertFilePath, std::string& fragFilePath);

    // https://docs.vulkan.org/guide/latest/storage_image_and_texel_buffers.html
    // transition outputImage from undefined to general
    void transitionImage(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
        VkImageAspectFlags aspectMask
	);

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
    uint32_t currentFrame{ 0 };

    bool outputImageInitialized{ false };
    bool accumulatedImageInitialized{ false };

    Vulkan::ComputePipeline computePipeline{};
    Vulkan::ComputeDescriptorSet computeDescriptorSet{};
    Vulkan::GraphicsPipeline graphicsPipeline{};
    Vulkan::GraphicsDescriptorSet graphicsDescriptorSet{};
    VkPipelineLayout graphicsPipelineLayout{ VK_NULL_HANDLE };

    Vulkan::Buffer sphereBuffer{};
    Vulkan::Buffer triangleBuffer{};
    Vulkan::Buffer materialBuffer{};
    Vulkan::Buffer bvhBuffer{};
    const Scene* scene{ nullptr };

    VkImage outputImage{ VK_NULL_HANDLE };
    VkImageView outputImageView{ VK_NULL_HANDLE };
    VmaAllocation outputImageAllocation{ VK_NULL_HANDLE };
    VkImage accumulatedImage{ VK_NULL_HANDLE };
    VkImageView accumulatedImageView{ VK_NULL_HANDLE };
    VmaAllocation accumulatedImageAllocation{ VK_NULL_HANDLE };
    VkImage depthImage{ VK_NULL_HANDLE };
    VkImageView depthImageView{ VK_NULL_HANDLE };
    VmaAllocation depthImageAllocation{ VK_NULL_HANDLE };

    uint32_t accumulatedFrames{ 0 };
    uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };
};
