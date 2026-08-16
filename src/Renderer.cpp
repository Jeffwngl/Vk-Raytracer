#include "Renderer.h"

#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

#include "Utils.h"

bool Renderer::initialize(VulkanCore& vkCore) {
    vulkanCore = &vkCore;
    
    // use separate image view from swapchain to avoid platform specifics
    // createOutputImage();
    // createOutputImageView();
    
    std::string path = "assets/shaders/color.comp.spv";
    
    // computeDescriptorSet.initialize(*vulkanCore, outputImageView);
    // computePipeline.initialize(*vulkanCore, path, computeDescriptorSet.getDescriptorSetLayout());

    return true;
}

// production version
/*
void Renderer::drawFrame() {
    FrameData& frame = vulkanCore->getFrameData(currentFrame);
    waitForFences(frame);

    uint32_t imageIndex;
    
    if (!acquireSwapchainImage(frame, imageIndex)) {
        return;
    }

    resetFences(frame);
    resetCommandBuffers(frame);

    recordCommandBuffers(
        frame.computeCommandBuffer,
        imageIndex
    );    
    
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
*/

// testing version
void Renderer::drawFrame() {
    FrameData& frame =
        vulkanCore->getFrameData(currentFrame);

    VkDevice device = vulkanCore->getDevice();
    VkQueue queue = vulkanCore->getQueue().get();

    // 1. Wait until this frame is free
    utils::check(vkWaitForFences(
        device,
        1,
        &frame.computeFence,
        VK_TRUE,
        UINT64_MAX
    ));

    // 2. Acquire a swapchain image
    uint32_t imageIndex;

    VkResult result = vkAcquireNextImageKHR(
        device,
        vulkanCore->getSwapchain(),
        UINT64_MAX,
        frame.imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }

    if (
        result != VK_SUCCESS &&
        result != VK_SUBOPTIMAL_KHR
    ) {
        throw std::runtime_error(
            "Failed to acquire swapchain image"
        );
    }

    // 3. Reuse the frame
    utils::check(vkResetFences(
        device,
        1,
        &frame.computeFence
    ));

    utils::check(vkResetCommandBuffer(
        frame.computeCommandBuffer,
        0
    ));

    // 4. Record compute + copy commands
    recordCommandBuffers(
        frame.computeCommandBuffer,
        imageIndex
    );

    // 5. Submit
    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_TRANSFER_BIT;

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.imageAvailable,
        .pWaitDstStageMask = &waitStage,

        .commandBufferCount = 1,
        .pCommandBuffers = &frame.computeCommandBuffer,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frame.renderFinished
    };

    utils::check(vkQueueSubmit(
        queue,
        1,
        &submitInfo,
        frame.computeFence
    ));

    // 6. Present
    VkSwapchainKHR swapchain =
        vulkanCore->getSwapchain();

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame.renderFinished,

        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex
    };

    utils::check(vkQueuePresentKHR(
        queue,
        &presentInfo
    ));

    // 7. Advance frame
    currentFrame =
        (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::createOutputImage() {
    VkImageCreateInfo imageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,

        .extent = {
            .width = static_cast<uint32_t>(vulkanCore->getWindowSize().x),
            .height = static_cast<uint32_t>(vulkanCore->getWindowSize().y),
            .depth = 1
        },

        .mipLevels = 1,
        .arrayLayers = 1,

        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,

        .usage =
            VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,

        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocationCI{
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    utils::check(vmaCreateImage(
        vulkanCore->getVmaAllocator(),
        &imageCI,
        &allocationCI,
        &outputImage,
        &outputImageAllocation,
        nullptr
    ));
}


void Renderer::createOutputImageView() {
    VkImageViewCreateInfo imageViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = outputImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    utils::check(vkCreateImageView(
        vulkanCore->getDevice(),
        &imageViewCI,
        nullptr,
        &outputImageView
    ));
}

void Renderer::waitForFences(FrameData& frame) {
    vkWaitForFences(
        vulkanCore->getDevice(),
        1,
        &frame.computeFence,
        VK_TRUE,
        UINT64_MAX
    );
/*
    vkWaitForFences(
        vulkanCore->getDevice(),
        1,
        &frame.graphicsFence,
        VK_TRUE,
        UINT64_MAX
    );
*/
}

bool Renderer::acquireSwapchainImage(FrameData& frame, uint32_t& imageIndex) {
    VkResult result = vkAcquireNextImageKHR(
        vulkanCore->getDevice(),
        vulkanCore->getSwapchain(),
        UINT64_MAX,
        frame.imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO: handle out of date case
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    return true;
}

void Renderer::resetFences(FrameData& frame) {
    vkResetFences(
        vulkanCore->getDevice(),
        1,
        &frame.computeFence
    );
/*
    vkResetFences(
        vulkanCore->getDevice(),
        1,
        &frame.graphicsFence
    );
*/
}

void Renderer::resetCommandBuffers(FrameData& frame) {
    vkResetCommandBuffer(frame.computeCommandBuffer, 0);
    // vkResetCommandBuffer(frame.graphicsCommandBuffer, 0);
}

/*
void Renderer::recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    utils::check(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkImage swapchainImage = vulkanCore->getSwapchainImages()[imageIndex];

    uint32_t width = static_cast<uint32_t>(vulkanCore->getWindowSize().x);
    uint32_t height = static_cast<uint32_t>(vulkanCore->getWindowSize().y);

    if (!outputImageInitialized) {
        transitionImage(
            commandBuffer,
            outputImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL
        );

        outputImageInitialized = true;
    };

    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        computePipeline.getPipeline()
    );

    VkDescriptorSet descriptorSet = computeDescriptorSet.getDescriptorSet();

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        computePipeline.getPipelineLayout(),
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

	vkCmdDispatch(
		commandBuffer,
		(width + 15) / 16,
		(height + 15) / 16,
		1
	);
    
    // after compute has finished writing
	transitionImage( // prepare output image
		commandBuffer,
		outputImage,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	);

	transitionImage( // prepare swapchain image for same format
		commandBuffer,
		swapchainImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	VkImageCopy region{
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
        }
	};
        
    // testing clear color

    VkClearColorValue clearColor{
        .float32 = { 0.1f, 0.3f, 0.8f, 1.0f }
    };

    VkImageSubresourceRange range{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    vkCmdClearColorImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clearColor,
        1,
        &range
    );


    vkCmdCopyImage(
        commandBuffer,
        outputImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // prepare for vkQueuePresentKHR
    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );
    
    transitionImage(
        commandBuffer,
        outputImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL
    );

    utils::check(vkEndCommandBuffer(commandBuffer));
}
*/

// testing clear color
void Renderer::recordCommandBuffers(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex
) {
    VkCommandBufferBeginInfo beginInfo{
        .sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    utils::check(
        vkBeginCommandBuffer(
            commandBuffer,
            &beginInfo
        )
    );

    VkImage swapchainImage =
        vulkanCore->getSwapchainImage(imageIndex);

    // Make acquired image writable by transfer commands.
    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    VkClearColorValue clearColor{
        .float32 = {
            0.1f,
            0.3f,
            0.8f,
            1.0f
        }
    };

    VkImageSubresourceRange range{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    vkCmdClearColorImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clearColor,
        1,
        &range
    );

    // Make it ready for presentation.
    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    );

    utils::check(
        vkEndCommandBuffer(commandBuffer)
    );
}

/*
void Renderer::submitCommandBuffers(){
	queue = vulkanCore->getQueue();	
	uint32_t imageIndex = queue->acquireNextImage();
	queue->submitAsync(vulkanCore->getFrameData(imageIndex).computeCommandBuffer);
	queue->present(imageIndex);
}
*/

void Renderer::transitionImage(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
) {
	VkImageMemoryBarrier2 barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.image = image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	
    VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    vkCmdPipelineBarrier2(
        commandBuffer,
        &dependencyInfo
    );
}

void Renderer::cleanUp() {
    if (outputImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(
            vulkanCore->getDevice(),
            outputImageView,
            nullptr
        );

        outputImageView = VK_NULL_HANDLE;
    }

    if (outputImage != VK_NULL_HANDLE) {
        vmaDestroyImage(
            vulkanCore->getVmaAllocator(),
            outputImage,
            outputImageAllocation
        );

        outputImage = VK_NULL_HANDLE;
        outputImageAllocation = VK_NULL_HANDLE;
    }
}

Renderer::~Renderer() {
    cleanUp();
}
