#include "Core.h"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Utils.h"

// #define NDEBUG

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

namespace Vulkan {

bool VulkanCore::initialize() {
    if (!std::filesystem::is_directory("assets")) {
        std::cerr << "Could not find assets folder from working director." << '\n';
        exit(-1);
    }

    createWindow();
    initializeInstance();
    setupDebugMessenger();
    createSurface();
    initializeDevice();
    createLogicalDevice();
    initializeVMA();
    createSwapchain();
    createCommandPool();
    createCommandBuffers();
    createSyncObjects();

    running = true;

    return true;
}

void VulkanCore::close() {
    running = false;
}

VmaAllocator VulkanCore::getVmaAllocator() const {
    return allocator;
}

VkDevice VulkanCore::getDevice() const {
    return this->device;
}

VkPhysicalDevice VulkanCore::getPhysicalDevice() const {
    return this->physicalDevice;
}

const Queue& VulkanCore::getQueue() const {
	return queue;
}

VkSurfaceKHR VulkanCore::getSurface() const {
    return this->surface;
}

const Swapchain& VulkanCore::getSwapchain() const {
    return swapchain;
}

FrameData& VulkanCore::getFrameData(uint32_t index) {
    return this->frames[index];
}

glm::vec2 VulkanCore::getWindowSize() const {
    return this->windowSize;
}

VkSemaphore VulkanCore::getRenderFinishedSemaphore(
    uint32_t imageIndex
) const {
    return renderFinishedSemaphores[imageIndex];
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
        .apiVersion = VulkanVersion,
        .pNext = VK_NULL_HANDLE
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
        .ppEnabledExtensionNames = extensions.data(),
        .pNext = VK_NULL_HANDLE
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
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = VK_NULL_HANDLE
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

        std::cout << "Selected Device: " << properties.deviceName << '\n';

        return;
    }

    throw std::runtime_error("Failed to find a suitable GPU.");
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

        if (
            (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && 
            (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
            supportsPresentation
        ) {
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

    queue.initialize(
        device,
        queueFamily,
        0
    );
}

void VulkanCore::initializeVMA() {
    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance,
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
    };

    utils::check(vmaCreateAllocator(&allocatorInfo, &allocator));
}

void VulkanCore::createSwapchain() {
    VkExtent2D windowExtent{
        .width = static_cast<uint32_t>(windowSize.x),
        .height = static_cast<uint32_t>(windowSize.y)
    };

    swapchain.initialize(
        *this,
        windowExtent
    );
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
        // frames[i].graphicsCommandBuffer = buffers[i * 2 + 1];
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
        frame.imageAvailable  = createSemaphore();

        frame.computeFence  = createFence();
	}

    renderFinishedSemaphores.resize(
        swapchain.getImages().size()
    );

    for (VkSemaphore& semaphore : renderFinishedSemaphores) {
        semaphore = createSemaphore();
    }
}

VkSemaphore VulkanCore::createSemaphore() {
	VkSemaphoreCreateInfo semaphoreCI{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.flags = 0
	};
	
	VkSemaphore semaphore;
	utils::check(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
	return semaphore;
}

VkFence VulkanCore::createFence() {
	VkFenceCreateInfo fenceCI{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,	
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VkFence fence;
	utils::check(vkCreateFence(device, &fenceCI, nullptr, &fence));
	return fence;
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
        vkDestroyFence(device, frame.computeFence, nullptr);
    }

    for (VkSemaphore semaphore : renderFinishedSemaphores) {
        vkDestroySemaphore(
            device,
            semaphore,
            nullptr
        );
    }

    swapchain.cleanUp();

    vkDestroyCommandPool(device, commandPool, nullptr);
    
    if (allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

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

}