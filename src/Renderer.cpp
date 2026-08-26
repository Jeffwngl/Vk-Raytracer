#include "Renderer.h"

#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

#include "Utils.h"

bool Renderer::initialize(Vulkan::VulkanCore& vkCore, const Scene& scene) {
    vulkanCore = &vkCore;
    this->scene = &scene;

    checkBlitSupport();
    
    // use separate image view from swapchain to avoid platform specifics
    createOutputImage();
    createOutputImageView();

    createSceneBuffer();
    
    std::string path = "assets/shaders/scene.comp.spv";

    createComputeDescriptorSet();
    createComputePipeline(path);

    return true;
}

void Renderer::drawFrame() {
    Vulkan::FrameData& frame = vulkanCore->getFrameData(currentFrame);

    VkDevice device = vulkanCore->getDevice();
    VkSwapchainKHR swapchain = vulkanCore->getSwapchain().get();    
    const Vulkan::Queue& queue = vulkanCore->getQueue();

    // 1. Wait until this frame is free
    waitForFences(device, frame);

    // 2. Acquire a swapchain image
    uint32_t imageIndex;
    
    if (!acquireSwapchainImage(device, swapchain, frame, imageIndex)) {
        return;
    }

    VkSemaphore renderFinished = vulkanCore->getRenderFinishedSemaphore(imageIndex);

    // 3. Reuse the frame
    resetFences(device, frame);
    resetCommandBuffers(frame);

    // 4. Record compute + copy commands
    recordCommandBuffers(
        frame.computeCommandBuffer,
        imageIndex
    );

    // 5. Submit 
    queue.submit(
        frame.computeCommandBuffer,
        frame.imageAvailable,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        renderFinished,
        frame.computeFence
    );

    // 6. Present 
    VkResult presentResult = queue.present(
        swapchain,
        imageIndex,
        renderFinished
    );

    if (
        presentResult != VK_SUCCESS &&
        presentResult != VK_SUBOPTIMAL_KHR &&
        presentResult != VK_ERROR_OUT_OF_DATE_KHR
    ) {
        throw std::runtime_error("Failed to present swapchain image!");
    }

    // 7. Advance frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

void Renderer::createSceneBuffer() {
    const std::vector<Sphere>& objects = scene->getObjects();

    if (objects.empty()) {
        throw std::runtime_error(
            "Cannot create scene buffer: Scene contains no objects."
        );
    }

    VkDeviceSize bufferSize = objects.size() * sizeof(Sphere);

    sceneObjectBuffer.initialize(
        vulkanCore->getVmaAllocator(),
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
        VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    sceneObjectBuffer.upload(
        objects.data(),
        bufferSize
    );
}

void Renderer::createComputeDescriptorSet() {
    computeDescriptorSet.initialize(
        *vulkanCore, 
        outputImageView,
        sceneObjectBuffer
    );
}

void Renderer::createComputePipeline(std::string& path) {
    computePipeline.initialize(
        *vulkanCore, 
        path, 
        computeDescriptorSet.getDescriptorSetLayout()
    );
}

void Renderer::waitForFences(VkDevice device, Vulkan::FrameData& frame) {
    utils::check(vkWaitForFences(
        device,
        1,
        &frame.computeFence,
        VK_TRUE,
        UINT64_MAX
    ));
}

bool Renderer::acquireSwapchainImage(VkDevice device, VkSwapchainKHR swapchain, Vulkan::FrameData& frame, uint32_t& imageIndex) {
    VkResult result = vkAcquireNextImageKHR(
        device,
        swapchain,
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

void Renderer::resetFences(VkDevice device, Vulkan::FrameData& frame) {
    utils::check(vkResetFences(
        device,
        1,
        &frame.computeFence
    ));
}

void Renderer::resetCommandBuffers(Vulkan::FrameData& frame) {
    utils::check(vkResetCommandBuffer(frame.computeCommandBuffer, 0));
}

void Renderer::recordCommandBuffers(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    utils::check(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkImage swapchainImage = vulkanCore->getSwapchain().getImages()[imageIndex];

    uint32_t width = static_cast<uint32_t>(vulkanCore->getWindowSize().x);
    uint32_t height = static_cast<uint32_t>(vulkanCore->getWindowSize().y);

    if (!outputImageInitialized) { // transition image if it is the first time it is used
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

    // push constants to tell GLSL how many objects to iterate over for ray tracing
    Vulkan::PushConstants pc{
        .objectCnt = static_cast<uint32_t>(scene->getObjects().size())
    };

    vkCmdPushConstants(
        commandBuffer,
        computePipeline.getPipelineLayout(),
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(Vulkan::PushConstants),
        &pc
    );

	vkCmdDispatch(
		commandBuffer,
		(width + 15) / 16,
		(height + 15) / 16,
		1
	);
    
	transitionImage( // compute result becomes copy source
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

    VkImageBlit region{
        .srcSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },

        .srcOffsets = {
            { 0, 0, 0 },
            {
                static_cast<int32_t>(width),
                static_cast<int32_t>(height),
                1
            }
        },

        .dstSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },

        .dstOffsets = {
            { 0, 0, 0 },
            {
                static_cast<int32_t>(width),
                static_cast<int32_t>(height),
                1
            }
        }
    };

    vkCmdBlitImage(
        commandBuffer,

        outputImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

        1,
        &region,

        VK_FILTER_NEAREST
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

void Renderer::checkBlitSupport() {
    VkPhysicalDevice physicalDevice = vulkanCore->getPhysicalDevice();

    VkFormat srcFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat dstFormat = vulkanCore->getSwapchain().getFormat();

    VkFormatProperties srcProperties;
    vkGetPhysicalDeviceFormatProperties(
        physicalDevice,
        srcFormat,
        &srcProperties
    );

    VkFormatProperties dstProperties;
    vkGetPhysicalDeviceFormatProperties(
        physicalDevice,
        dstFormat,
        &dstProperties
    );

    if (!(srcProperties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        throw std::runtime_error(
            "Output image format does not support blit source"
        );
    }

    if (!(dstProperties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        throw std::runtime_error(
            "Swapchain format does not support blit destination"
        );
    }
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
    if (vulkanCore != nullptr) {
        vkDeviceWaitIdle(vulkanCore->getDevice());
    };

    cleanUp();
}
