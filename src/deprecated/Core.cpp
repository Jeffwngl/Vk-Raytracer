#include <cstddef>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <limits>

#include "Core.h"

#include "Utils.h"

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    std::vector<VkSurfaceFormatKHR>formats;
    std::vector<VkPresentModeKHR>presentModes;
};

struct FrameData {
    VkSemaphore imageAvailable{ VK_NULL_HANDLE };
    VkSemaphore computeFinished{ VK_NULL_HANDLE };
    VkSemaphore renderFinished{ VK_NULL_HANDLE };

    VkFence computeFence{ VK_NULL_HANDLE };
    VkFence graphicsFence{ VK_NULL_HANDLE };

    VkCommandBuffer computeCommandBuffer{ VK_NULL_HANDLE };
    VkCommandBuffer graphicsCommandBuffer{ VK_NULL_HANDLE };
};

class VulkanCore {
public:
    bool initialize(uint32_t deviceNum) {
        if (!std::filesystem::is_directory("assets")) {
            std::cerr << "Could not find assets folder from working director." << '\n';
            exit(-1);
        }
        createWindow();
        initializeInstance();
        createSurface();
        initializeDevice(deviceNum);
        createLogicalDevice();
        createSwapChain();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        
        return true;
    }

    ~VulkanCore() {
        cleanUp();
    }

private:
    void createWindow() {
        utils::check(SDL_Init(SDL_INIT_VIDEO));
        utils::check(SDL_Vulkan_LoadLibrary(NULL));
        window = SDL_CreateWindow("Raytracer", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        assert(window);
    }

    void initializeInstance() {
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Vulkan Raytracer",
            .apiVersion = VK_API_VERSION_1_3,
        };

        uint32_t instanceExtensionCnt{ 0 };
        char const* const* instanceExpressions{ SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCnt) };

        VkInstanceCreateInfo instanceCI{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = instanceExtensionCnt,
            .ppEnabledExtensionNames = instanceExpressions
        };

        utils::check(vkCreateInstance(&instanceCI, nullptr, &instance));
    }

    void createSurface() {
        utils::check(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
        utils::check(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
    }

    void initializeDevice(uint32_t deviceNum) {
        uint32_t deviceCnt{ 0 };
        utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, nullptr));

        if (deviceCnt == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");
        };

        std::vector<VkPhysicalDevice>devices(deviceCnt);
        utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, devices.data()));

        // manually choose device
        assert(deviceNum < deviceCnt);
        
        VkPhysicalDeviceProperties2 deviceProperties{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };

        vkGetPhysicalDeviceProperties2(devices[deviceNum], &deviceProperties);
        physicalDevice = devices[deviceNum];
        std::cout << "Selected Device: " << deviceProperties.properties.deviceName << '\n';
    }

    void createLogicalDevice() {
        uint32_t queueFamilyCnt{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCnt, nullptr);
        std::vector<VkQueueFamilyProperties>queueFamilies(queueFamilyCnt);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCnt, queueFamilies.data());
        
        for (size_t i = 0; i < queueFamilyCnt; ++i) {
            const bool supportsPresentation =
            SDL_Vulkan_GetPresentationSupport(
                instance,
                physicalDevice,
                i
            );

            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresentation) { // vkCmdDraw
                queueFamily = i;
                break;
            }
        }
        utils::check(SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, queueFamily));

        const float queuePriority{ 1.0f };
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        VkPhysicalDeviceVulkan13Features supportedVk13{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
        };

        VkPhysicalDeviceFeatures2 deviceFeatures {
            // TODO: add features as needed
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &supportedVk13
        };

        vkGetPhysicalDeviceFeatures2(
            physicalDevice,
            &deviceFeatures
        );

        if (!supportedVk13.dynamicRendering || !supportedVk13.synchronization2) {
            throw std::runtime_error("Required Vulkan 1.3 features are unsupported");
        }

        VkPhysicalDeviceVulkan12Features enabledVk12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
            // TODO: add features as needed
        };
        VkPhysicalDeviceVulkan13Features enabledVk13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
            // TODO: add features as needed
            // enable .dynamicRendering for dynamic rendering
            .dynamicRendering = VK_TRUE,
            .synchronization2 = VK_TRUE
        };

        enabledVk12Features.pNext = &enabledVk13Features;

        const std::vector<const char*>requiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &enabledVk12Features,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCI,
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data()
            // TODO: .pNext and pEnabledFeatures to add
        };
        utils::check(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
        vkGetDeviceQueue(device, queueFamily, 0, &graphicsQueue);
    }

    void initializeVMA() {
        
    }

    void createSwapChain() {
        SwapchainSupportDetails supportDetails = querySwapChainSupport(physicalDevice, surface);

        uint32_t swapFormat{ 0 };
        for (size_t i = 0; i < supportDetails.formats.size(); ++i) { // TODO: offload this to separate function
            if (supportDetails.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && supportDetails.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                swapFormat = i;
                break;
            }
        }
        swapChainFormat = supportDetails.formats[swapFormat].format;

        VkPresentModeKHR swapPresentMode{};
        for (size_t i = 0; i < supportDetails.presentModes.size(); ++i) {
            if (supportDetails.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                swapPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
        if (swapPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
            swapPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

        VkExtent2D swapchainExtent = chooseSwapExtent(surfaceCaps);

        uint32_t imageCnt = surfaceCaps.minImageCount + 1;
        if (surfaceCaps.maxImageCount > 0 && imageCnt > surfaceCaps.maxImageCount) {
            imageCnt = surfaceCaps.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCI{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = imageCnt,
                .imageFormat = supportDetails.formats[swapFormat].format,
                .imageColorSpace = supportDetails.formats[swapFormat].colorSpace,
                .imageExtent{ .width = swapchainExtent.width, .height = swapchainExtent.height },
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .preTransform = surfaceCaps.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = swapPresentMode,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE
        };
        utils::check(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));

        utils::check(vkGetSwapchainImagesKHR(device, swapchain, &imageCnt, nullptr));
        swapChainImages.resize(imageCnt);
        utils::check(vkGetSwapchainImagesKHR(device, swapchain, &imageCnt, swapChainImages.data()));

        swapChainImageViews.resize(imageCnt);
        for (size_t i = 0; i < swapChainImages.size(); ++i) {
            VkImageViewCreateInfo imageViewCI{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = supportDetails.formats[swapFormat].format,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.levelCount = 1,
                .subresourceRange.layerCount = 1
            };
            utils::check(vkCreateImageView(device, &imageViewCI, nullptr, &swapChainImageViews[i]));
        }
    }

    /**
     * Note that the code below is only for Vulkan version < 1.3
     * where dynamic rendering wasn't added, in these cases, we
     * would have to specify the renderpass, framebuffer, subpass
     * description and subpass dependency.
     *
     * The current approach uses dynamic rendering.
     */

    void createRenderPass() { 
        VkAttachmentDescription colorAttachment{
            .flags = 0,
            .format = swapChainFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };


        VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,

            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

            .dependencyFlags = 0,
        };

        VkRenderPassCreateInfo renderPassCI{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency,
        };
 
        utils::check(vkCreateRenderPass(device, &renderPassCI, nullptr, &renderPass));
    }

    /**
     * The code below is the same for Vulkan >= 1.3 or Vulkan < 1.3
     */

	void createCommandPool() {
		VkCommandPoolCreateInfo poolInfo{
		    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		    .queueFamilyIndex = queueFamily
        };

        utils::check(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
    }

	void createCommandBuffers() {
        /* Depracated
         *
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocCI{
		    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		    .commandPool = commandPool,
		    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		    .commandBufferCount = (uint32_t)commandBuffers.size()
        };

        utils::check(vkAllocateCommandBuffers(device, &allocCI, commandBuffers.data()));
        */

        std::vector<VkCommandBuffer> buffers(MAX_FRAMES_IN_FLIGHT * 2);
        frames.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(buffers.size()),
        };
        utils::check(vkAllocateCommandBuffers(device, &allocCI, buffers.data()));

        for (size_t i = 0; i < frames.size(); ++i) {
            frames[i].computeCommandBuffer = buffers[i * 2];
            frames[i].graphicsCommandBuffer = buffers[i * 2 + 1];
        }
	}

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreCI{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        VkFenceCreateInfo fenceCI{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        /* Depracated

        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imageAcquiredSempahores.resize(MAX_FRAMES_IN_FLIGHT);

        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            utils::check(vkCreateFence(device, &fenceCI, nullptr, &inFlightFences[i]));
            utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &imageAcquiredSempahores[i]));
        }

        renderCompletedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        for (auto& semaphore : renderCompletedSemaphores) {
            utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, semaphore));
        }
        */
        
        for (FrameData& frame : frames) {
            utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &frame.imageAvailable));
            utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &frame.computeFinished));
            utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &frame.renderFinished));
            utils::check(vkCreateFence(device, &fenceCI, nullptr, &frame.computeFence));
            utils::check(vkCreateFence(device, &fenceCI, nullptr, &frame.graphicsFence));
        }
    }

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        SwapchainSupportDetails details{};

        utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.surfaceCapabilities));

        uint32_t formatCnt{ 0 };
        utils::check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCnt, nullptr));
        if (formatCnt > 0) {
            details.formats.resize(formatCnt);
            utils::check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCnt, details.formats.data()));
        }

        uint32_t presentModeCnt{ 0 };
        utils::check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCnt, nullptr));
        if (presentModeCnt > 0) {
            details.presentModes.resize(presentModeCnt);
            utils::check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCnt,details.presentModes.data()));
        }

        return details;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCaps) {
		if (surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return surfaceCaps.currentExtent;
		}
        else {
            VkExtent2D swapExtent = {};
            swapExtent.width = std::clamp(
                static_cast<uint32_t>(windowSize.x),
                surfaceCaps.minImageExtent.width,
                surfaceCaps.maxImageExtent.width
            );

            swapExtent.height = std::clamp(
                static_cast<uint32_t>(windowSize.y),
                surfaceCaps.minImageExtent.height,
                surfaceCaps.maxImageExtent.height
            );
            
            return swapExtent;
        }
    }

    void cleanUp() {
        vkDeviceWaitIdle(device);

        for (FrameData& frame : frames) {
            vkDestroySemaphore(device, frame.imageAvailable, nullptr);
            vkDestroySemaphore(device, frame.computeFinished, nullptr);
            vkDestroySemaphore(device, frame.renderFinished, nullptr);

            vkDestroyFence(device, frame.computeFence, nullptr);
            vkDestroyFence(device, frame.graphicsFence, nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        for (VkImageView view : swapChainImageViews) {
            vkDestroyImageView(device, view, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        SDL_DestroyWindow(window);
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();
    }

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };

    VkInstance instance{ VK_NULL_HANDLE };
    VkQueue graphicsQueue{ VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
    SDL_Window* window{};
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    VkFormat swapChainFormat{ VK_FORMAT_UNDEFINED };
    VkRenderPass renderPass{ VK_NULL_HANDLE };
    VkCommandPool commandPool{ VK_NULL_HANDLE };

    std::vector<VkImage>swapChainImages{};
    std::vector<VkImageView>swapChainImageViews{};
    // std::vector<VkCommandBuffer>commandBuffers{};
    std::vector<FrameData>frames{};
    // std::vector<VkFence>inFlightFences{};
    // std::vector<VkSemaphore>imageAcquiredSempahores{};
    // std::vector<VkSemaphore>renderCompletedSemaphores{};

    uint32_t queueFamily{ 0 };

    glm::ivec2 windowSize{};

};
