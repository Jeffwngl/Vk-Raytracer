#include "Core.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <SDL3/SDL_vulkan.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Utils.h"

#define NDEBUG

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

bool VulkanCore::initialize(uint32_t deviceNum) {
    if (!std::filesystem::is_directory("assets")) {
        std::cerr << "Could not find assets folder from working director." << '\n';
        exit(-1);
    }

    createWindow();
    initializeInstance();
    createSurface();
    initializeDevice(deviceNum);
    createLogicalDevice();
    initializeVMA();
    createSwapChain();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();

    running = true;

    return true;
}

void VulkanCore::close() {
    running = false;
}

VkDevice VulkanCore::getDevice() const {
    return this->device;
}

VkQueue VulkanCore::getQueue() const {
    return this->graphicsQueue;
}

VkSwapchainKHR VulkanCore::getSwapchain() const {
    return this->swapchain;
}

const std::vector<VkImage>& VulkanCore::getSwapchainImages() const {
    return this->swapChainImages;
}

const std::vector<VkImageView>& VulkanCore::getSwapchainImageViews() const {
    return this->swapChainImageViews;
}

FrameData& VulkanCore::getFrameData(uint32_t index) {
    return this->frames[index];
}

glm::vec2 VulkanCore::getWindowSize() const {
    return this->windowSize;
}

void VulkanCore::createWindow() {
    utils::check(SDL_Init(SDL_INIT_VIDEO));
    utils::check(SDL_Vulkan_LoadLibrary(NULL));

    window = SDL_CreateWindow(
        "Raytracer",
        1280u,
        720u,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    assert(window);
}

void VulkanCore::initializeInstance() {
    if (enableValidationLayers &&
        !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "Validation layers requested but unavailable."
        );
    }

    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Raytracer",
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t sdlExtensionCnt{ 0 }; // SDL required extensions
    char const* const* sdlExtensions{
        SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCnt)
    };

    std::vector<const char*> extensions(
        sdlExtensions,
        sdlExtensions + sdlExtensionCnt
    );

    if (enableValidationLayers) {
        extensions.push_back(
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        );
    }

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data() 
    };

    if (enableValidationLayers) {
        instanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCI.ppEnabledLayerNames = validationLayers.data();
    } 
    else {
        instanceCI.enabledLayerCount = 0;
    }

    utils::check(vkCreateInstance(&instanceCI, nullptr, &instance));
}


void VulkanCore::createSurface() {
    utils::check(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
    utils::check(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
}


void VulkanCore::initializeDevice() {
    uint32_t deviceCnt{ 0 };
    utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, nullptr));

    if (deviceCnt == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCnt);
    utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, devices.data()));

    VkPhysicalDeviceProperties2 deviceProperties{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
    };

    for (VkPhysicalDevice device : devices) {
        if (!checkSuitableDevice(device)) {
            continue;
        }

        physicalDevice = device;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(
            physicalDevice,
            &properties
        );

        std::cout
            << "Selected Device: "
            << properties.deviceName
            << '\n';

        return;
    }

    throw std::runtime_error(
        "Failed to find a suitable GPU."
    );
}

void VulkanCore::createLogicalDevice() {
    uint32_t queueFamilyCnt{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCnt, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCnt);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,
        &queueFamilyCnt,
        queueFamilies.data()
    );

    for (size_t i = 0; i < queueFamilyCnt; ++i) {
        const bool supportsPresentation =
            SDL_Vulkan_GetPresentationSupport(
                instance,
                physicalDevice,
                i
            );

        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresentation) {
            queueFamily = i;
            break;
        }
    }

    utils::check(SDL_Vulkan_GetPresentationSupport(
        instance,
        physicalDevice,
        queueFamily
    ));

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

    VkPhysicalDeviceFeatures2 deviceFeatures{
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
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = nullptr,
        .bufferDeviceAddress = VK_TRUE,
        .descriptorIndexing = VK_TRUE
    };

    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE,
        .synchronization2 = VK_TRUE
    };

    enabledVk12Features.pNext = &enabledVk13Features;

    const std::vector<const char*> requiredDeviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk12Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = requiredDeviceExtensions.data()
    };

    utils::check(vkCreateDevice(
        physicalDevice,
        &deviceCI,
        nullptr,
        &device
    ));

    vkGetDeviceQueue(device, queueFamily, 0, &graphicsQueue);
}


void VulkanCore::initializeVMA() {
    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance,
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);
}


void VulkanCore::createSwapChain() {
    SwapchainSupportDetails supportDetails =
        querySwapChainSupport(physicalDevice, surface);

    uint32_t swapFormat{ 0 };

    for (size_t i = 0; i < supportDetails.formats.size(); ++i) {
        if (
            supportDetails.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            supportDetails.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ) {
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
    utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &surfaceCaps
    ));

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
        .imageExtent{
            .width = swapchainExtent.width,
            .height = swapchainExtent.height
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    utils::check(vkCreateSwapchainKHR(
        device,
        &swapchainCI,
        nullptr,
        &swapchain
    ));

    utils::check(vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &imageCnt,
        nullptr
    ));

    swapChainImages.resize(imageCnt);

    utils::check(vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &imageCnt,
        swapChainImages.data()
    ));

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

        utils::check(vkCreateImageView(
            device,
            &imageViewCI,
            nullptr,
            &swapChainImageViews[i]
        ));
    }
}


void VulkanCore::createRenderPass() {
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

    utils::check(vkCreateRenderPass(
        device,
        &renderPassCI,
        nullptr,
        &renderPass
    ));
}


void VulkanCore::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily
    };

    utils::check(vkCreateCommandPool(
        device,
        &poolInfo,
        nullptr,
        &commandPool
    ));
}


void VulkanCore::createCommandBuffers() {
    std::vector<VkCommandBuffer> buffers(MAX_FRAMES_IN_FLIGHT * 2);
    frames.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocCI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(buffers.size()),
    };

    utils::check(vkAllocateCommandBuffers(
        device,
        &allocCI,
        buffers.data()
    ));

    for (size_t i = 0; i < frames.size(); ++i) {
        frames[i].computeCommandBuffer = buffers[i * 2];
        frames[i].graphicsCommandBuffer = buffers[i * 2 + 1];
    }
}


void VulkanCore::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreCI{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceCI{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (FrameData& frame : frames) {
        utils::check(vkCreateSemaphore(
            device,
            &semaphoreCI,
            nullptr,
            &frame.imageAvailable
        ));

        utils::check(vkCreateSemaphore(
            device,
            &semaphoreCI,
            nullptr,
            &frame.computeFinished
        ));

        utils::check(vkCreateSemaphore(
            device,
            &semaphoreCI,
            nullptr,
            &frame.renderFinished
        ));

        utils::check(vkCreateFence(
            device,
            &fenceCI,
            nullptr,
            &frame.computeFence
        ));

        utils::check(vkCreateFence(
            device,
            &fenceCI,
            nullptr,
            &frame.graphicsFence
        ));
    }
}

bool VulkanCore::checkSuitableDevice(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool isGPU =
        deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
        deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

    bool supportsRequiredFeatures =
        deviceFeatures.samplerAnisotropy;

    return isGPU && supportsRequiredFeatures;
}

SwapchainSupportDetails VulkanCore::querySwapChainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface
) {
    SwapchainSupportDetails details{};

    utils::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &details.surfaceCapabilities
    ));

    uint32_t formatCnt{ 0 };

    utils::check(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCnt,
        nullptr
    ));

    if (formatCnt > 0) {
        details.formats.resize(formatCnt);

        utils::check(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCnt,
            details.formats.data()
        ));
    }

    uint32_t presentModeCnt{ 0 };

    utils::check(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCnt,
        nullptr
    ));

    if (presentModeCnt > 0) {
        details.presentModes.resize(presentModeCnt);

        utils::check(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCnt,
            details.presentModes.data()
        ));
    }

    return details;
}


VkExtent2D VulkanCore::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& surfaceCaps
) {
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

bool VulkanCore::checkValidationLayerSupport() {
    uint32_t layerCount;

    vkEnumerateInstanceLayerProperties(
        &layerCount,
        nullptr
    );

    std::vector<VkLayerProperties> availableLayers(layerCount);

    vkEnumerateInstanceLayerProperties(
        &layerCount,
        availableLayers.data()
    );

    for (const char* layerName : validationLayers) {
        bool found = false;

        for (const auto& layer : availableLayers) {
            if (strcmp(layerName, layer.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanCore::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    std::cerr
        << "[Vulkan Validation] "
        << callbackData->pMessage
        << '\n';

    return VK_FALSE;
}

void VulkanCore::setupDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCI{
        .sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,

        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

        .messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

        .pfnUserCallback = debugCallback,
    };

    auto createDebugMessenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(
                instance,
                "vkCreateDebugUtilsMessengerEXT"
            )
        );

    if (!createDebugMessenger) {
        throw std::runtime_error(
            "Failed to find vkCreateDebugUtilsMessengerEXT."
        );
    }

    utils::check(
        createDebugMessenger(
            instance,
            &debugCI,
            nullptr,
            &debugMessenger
        )
    );
}

void VulkanCore::destroyDebugMessenger() {
    if (!enableValidationLayers ||
        debugMessenger == VK_NULL_HANDLE) {
        return;
    }

    auto destroyDebugMessenger =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(
                instance,
                "vkDestroyDebugUtilsMessengerEXT"
            )
        );

    if (destroyDebugMessenger) {
        destroyDebugMessenger(
            instance,
            debugMessenger,
            nullptr
        );
    }

    debugMessenger = VK_NULL_HANDLE;
}

void VulkanCore::cleanUp() {
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
    destroyDebugMessenger();
    vkDestroyInstance(instance, nullptr);

    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

VulkanCore::~VulkanCore() {
    cleanUp();
}
