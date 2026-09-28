#include "Renderer.h"

#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

#include "Utils.h"
#include "Camera.h"

bool Renderer::initialize(Vulkan::VulkanCore& vkCore, const Scene& scene) {
    vulkanCore = &vkCore;
    this->scene = &scene;
    
    // use separate image view from swapchain to avoid platform specifics
    createImages();
    createImageViews();

    createBuffers();
    
    std::string raytracePath = "assets/shaders/raytrace.comp.spv";
    std::string vertPath = "assets/shaders/vert.comp.spv";
    std::string fragPath = "assets/shaders/frag.comp.spv";

    createComputeDescriptorSet();
    createComputePipeline(raytracePath);

    createGraphicsDescriptorSet();
    createGraphicsPipelineLayout();
    createGraphicsPipeline(vertPath, fragPath);

    return true;
}

void Renderer::drawFrame(ImGuiLayer& imgui, RenderSettings& settings, DebugSettings& debug) {
    Vulkan::FrameData& frame = vulkanCore->getFrameData(currentFrame);

    VkDevice device = vulkanCore->getDevice().get();
    VkSwapchainKHR swapchain = vulkanCore->getSwapchain().get();    
    const Vulkan::Queue& queue = vulkanCore->getDevice().getQueue();

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
        imageIndex,
        imgui,
        settings,
        debug
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
        throw std::runtime_error("Failed to present swapchain image.");
    }

    // 7. Advance frame
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

uint32_t Renderer::getAccumulatedFrames() const {
    return accumulatedFrames;
}

void Renderer::advanceAccumulatedFrames() {
    accumulatedFrames++;
};

void Renderer::resetAccumulatedFrames() {
    accumulatedFrames = 0;
}

void Renderer::createImages() {
    createOutputImage();
    createAccumulatedImage();
    createDepthImage();
}

void Renderer::createImageViews() {
    createOutputImageView();
    createAccumulatedImageView();
    createDepthImageView();
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

void Renderer::createAccumulatedImage() {
    VkImageCreateInfo imageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
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
            VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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
        &accumulatedImage,
        &accumulatedImageAllocation,
        nullptr
    ));
}

void Renderer::createDepthImage() {
    VkImageCreateInfo imageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .extent = {
            .width = static_cast<uint32_t>(vulkanCore->getWindowSize().x),
            .height = static_cast<uint32_t>(vulkanCore->getWindowSize().y),
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocationCI{
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    utils::check(
        vmaCreateImage(
            vulkanCore->getVmaAllocator(),
            &imageCI,
            &allocationCI,
            &depthImage,
            &depthImageAllocation,
            nullptr
        )
    );
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
        vulkanCore->getDevice().get(),
        &imageViewCI,
        nullptr,
        &outputImageView
    ));
}

void Renderer::createAccumulatedImageView() {
    VkImageViewCreateInfo imageViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = accumulatedImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R16G16B16A16_SFLOAT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    utils::check(vkCreateImageView(
        vulkanCore->getDevice().get(),
        &imageViewCI,
        nullptr,
        &accumulatedImageView
    ));
}

void Renderer::createDepthImageView() {
    VkImageViewCreateInfo imageViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    utils::check(
        vkCreateImageView(
            vulkanCore->getDevice().get(),
            &imageViewCI,
            nullptr,
            &depthImageView
        )
    );
}

void Renderer::createBuffers() {
    createStorageBuffer(sphereBuffer, scene->getSpheres());
    createStorageBuffer(triangleBuffer, scene->getTriangles());
    createStorageBuffer(materialBuffer, scene->getMaterials());
    createStorageBuffer(bvhBuffer, scene->getBVH().getNodes());
}

template<typename T>
void Renderer::createStorageBuffer(
    Vulkan::Buffer& buffer,
    const std::vector<T>& data
) {
    VkDeviceSize bufferSize = data.empty() ? sizeof(T) : data.size() * sizeof(T);

    buffer.initialize(
        vulkanCore->getVmaAllocator(),
        bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT
    );

    if (!data.empty()) {
        buffer.upload(
            data.data(),
            bufferSize
        );
    }
}

void Renderer::createComputeDescriptorSet() {
    computeDescriptorSet.initialize(
        *vulkanCore, 
        outputImageView,
        accumulatedImageView,
        sphereBuffer,
        triangleBuffer,
        materialBuffer,
        bvhBuffer
    );
}

void Renderer::createComputePipeline(std::string& path) {
    computePipeline.initialize(
        *vulkanCore, 
        path, 
        computeDescriptorSet.getDescriptorSetLayout()
    );
}

void Renderer::createGraphicsDescriptorSet() {
    graphicsDescriptorSet.initialize(
        *vulkanCore,
        triangleBuffer
    );
}

void Renderer::createGraphicsPipelineLayout() {
    VkDescriptorSetLayout setLayout = graphicsDescriptorSet.getDescriptorSetLayout();

    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(Vulkan::GraphicsPushConstants)
    };

    VkPipelineLayoutCreateInfo layoutCI{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &setLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    utils::check(
        vkCreatePipelineLayout(
            vulkanCore->getDevice().get(),
            &layoutCI,
            nullptr,
            &graphicsPipelineLayout
        )
    );
}

void Renderer::createGraphicsPipeline(
    std::string& vertFilePath, 
    std::string& fragFilePath
) {
    PipelineConfigInfo config = graphicsPipeline.defaultPipelineConfigInfo();

    config.pipelineLayout = graphicsPipelineLayout;
    config.colorFormat = vulkanCore->getSwapchain().getFormat();
    config.depthFormat = VK_FORMAT_D32_SFLOAT;

    graphicsPipeline.initialize(
        vulkanCore->getDevice().get(),
        "assets/shaders/debug.vert.spv",
        "assets/shaders/debug.frag.spv",
        config
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

void Renderer::recordCommandBuffers(
    VkCommandBuffer commandBuffer, 
    uint32_t imageIndex,
    ImGuiLayer& imgui,
    RenderSettings& settings,
    DebugSettings& debug
) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    utils::check(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	VkImage swapchainImage = vulkanCore->getSwapchain().getImages()[imageIndex];

    uint32_t width = static_cast<uint32_t>(vulkanCore->getWindowSize().x);
    uint32_t height = static_cast<uint32_t>(vulkanCore->getWindowSize().y);

    if (debug.viewMode == ViewMode::Raytrace) {
        recordRaytraceCommands(
            commandBuffer,
            imageIndex,
            settings,
            debug,
            width,
            height,
            imgui
        );
    }
    else {
        recordDebugCommands(
            commandBuffer,
            imageIndex,
            settings,
            debug,
            width,
            height,
            imgui
        );
    }

    // prepare for vkQueuePresentKHR
    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    utils::check(vkEndCommandBuffer(commandBuffer));
}

void Renderer::recordRaytraceCommands(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    RenderSettings& settings,
    DebugSettings& debug,
    uint32_t width,
    uint32_t height,
    ImGuiLayer& imgui
) {
	VkImage swapchainImage = vulkanCore->getSwapchain().getImages()[imageIndex];

    if (!outputImageInitialized) { // transition image if it is the first time it is used
        transitionImage(
            commandBuffer,
            outputImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        outputImageInitialized = true;
    };

    if (!accumulatedImageInitialized) {
        transitionImage(
            commandBuffer,
            accumulatedImage,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_ASPECT_COLOR_BIT
        );

        accumulatedImageInitialized = true;
    }

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

    Camera camera = scene->getCamera();

    Vulkan::PushConstants pc{
        .camera = camera.getGPUData(width, height),
        .sphereCnt = static_cast<uint32_t>(scene->getSpheres().size()),
        .triangleCnt = static_cast<uint32_t>(scene->getTriangles().size()),
        .samplesPerPixel = settings.samplesPerPixel,
        .maxBounces = settings.maxBounces,
        .accumulatedFrames = accumulatedFrames,
        .accumulateRays = settings.accumulateRays,
        .viewMode = static_cast<uint32_t>(debug.viewMode),
        .bvhDepth = debug.bvhDepth
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
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT
	);

	transitionImage( // prepare swapchain image for same format
		commandBuffer,
		swapchainImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT
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

    // transfer destination -> graphics color attachment
    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkImageView swapchainImageView = vulkanCore->getSwapchain().getImageViews()[imageIndex];

    VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchainImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .offset = {0, 0},
            .extent = {width, height}
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );

    // render ImGui here
    imgui.render(commandBuffer);

    vkCmdEndRendering(commandBuffer);

    transitionImage(
        commandBuffer,
        outputImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_ASPECT_COLOR_BIT
    );
}

void Renderer::recordDebugCommands(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    RenderSettings& settings,
    DebugSettings& debug,
    uint32_t width,
    uint32_t height,
    ImGuiLayer& imgui
) {
    VkImage swapchainImage = vulkanCore->getSwapchain().getImages()[imageIndex];
    VkImageView swapchainImageView = vulkanCore->getSwapchain().getImageViews()[imageIndex];

    transitionImage(
        commandBuffer,
        swapchainImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkRenderingAttachmentInfo colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchainImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .color = {{0.05f, 0.05f, 0.05f, 1.0f}}
        }
    };

    VkRenderingAttachmentInfo depthAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depthImageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .depthStencil = {
                1.0f,
                0
            }
        }
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .offset = { 0, 0 },
            .extent = {width, height}
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment
    }; 

    transitionImage(
        commandBuffer,
        depthImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );

    // bind raster graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline.getPipeline()
    );

    VkDescriptorSet descriptorSet = graphicsDescriptorSet.getDescriptorSet();

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    // viewport
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(width),
        .height = static_cast<float>(height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = { 0, 0 },
        .extent = {width, height}
    };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // camera matrices
    Camera camera = scene->getCamera();

    glm::mat4 view = glm::lookAt(
        camera.getPos(),
        camera.getTarget(),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(camera.getFov()),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        1000.0f
    );

    // vulkan clip-space y correction
    projection[1][1] *= -1.0f;

    Vulkan::GraphicsPushConstants pc{
        .viewProjection = projection * view
    };

    vkCmdPushConstants(
        commandBuffer,
        graphicsPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(Vulkan::GraphicsPushConstants),
        &pc
    );

    // draw triangles
    vkCmdDraw(
        commandBuffer,
        static_cast<uint32_t>(scene->getTriangles().size() * 3),
        1,
        0,
        0
    );

    vkCmdEndRendering(commandBuffer);

    // begin second rendering for imgui because of dumbass vulkan/imgui imageView warning
    VkRenderingAttachmentInfo imguiColorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchainImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        // keep rasterized scene already drawn
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
    };

    VkRenderingInfo imguiRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .offset = {0, 0},
            .extent = {width, height}
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &imguiColorAttachment,
        // imgui complains about this
        .pDepthAttachment = nullptr
    };

    vkCmdBeginRendering(
        commandBuffer,
        &imguiRenderingInfo
    );

    imgui.render(commandBuffer);

    vkCmdEndRendering(commandBuffer);
}

void Renderer::transitionImage(
		VkCommandBuffer commandBuffer,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
        VkImageAspectFlags aspectMask
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
			.aspectMask = aspectMask,
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
            vulkanCore->getDevice().get(),
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

    if (accumulatedImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(
            vulkanCore->getDevice().get(),
            accumulatedImageView,
            nullptr
        );

        accumulatedImageView = VK_NULL_HANDLE;
    }

    if (accumulatedImage != VK_NULL_HANDLE) {
        vmaDestroyImage(
            vulkanCore->getVmaAllocator(),
            accumulatedImage,
            accumulatedImageAllocation
        );

        accumulatedImage = VK_NULL_HANDLE;
        accumulatedImageAllocation = VK_NULL_HANDLE;
    }

    if (depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(
            vulkanCore->getDevice().get(),
            depthImageView,
            nullptr
        );

        depthImageView = VK_NULL_HANDLE;
    }

    if (depthImage != VK_NULL_HANDLE) {
        vmaDestroyImage(
            vulkanCore->getVmaAllocator(),
            depthImage,
            depthImageAllocation
        );

        depthImage = VK_NULL_HANDLE;
        depthImageAllocation = VK_NULL_HANDLE;
    }

    if (graphicsPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(
            vulkanCore->getDevice().get(),
            graphicsPipelineLayout,
            nullptr
        );

        graphicsPipelineLayout =
            VK_NULL_HANDLE;
    }
}

Renderer::~Renderer() {
    if (vulkanCore != nullptr) {
        vkDeviceWaitIdle(vulkanCore->getDevice().get());
    };

    cleanUp();
}
