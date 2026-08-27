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

Device VulkanCore::getDevice() const {
    return device;
}

VkSurfaceKHR VulkanCore::getSurface() const {
    return surface;
}

const Swapchain& VulkanCore::getSwapchain() const {
    return swapchain;
}

FrameData& VulkanCore::getFrameData(uint32_t index) {
    return frames[index];
}

glm::vec2 VulkanCore::getWindowSize() const {
    return windowSize;
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
    device.initialize(
        instance,
        surface
    );
}

void VulkanCore::initializeVMA() {
    VmaAllocatorCreateInfo allocatorInfo = {
        .physicalDevice = device.getPhysicalDevice(),
        .device = device.get(),
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
        .queueFamilyIndex = device.getQueueFamily()
    };

    utils::check(vkCreateCommandPool(
        device.get(),
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
        device.get(),
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
	utils::check(vkCreateSemaphore(
        device.get(), 
        &semaphoreCI, 
        nullptr, 
        &semaphore
    ));

	return semaphore;
}

VkFence VulkanCore::createFence() {
	VkFenceCreateInfo fenceCI{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,	
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VkFence fence;
	utils::check(vkCreateFence(
        device.get(), 
        &fenceCI, 
        nullptr, 
        &fence
    ));

	return fence;
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
    VkDevice vkDevice = device.get();

    vkDeviceWaitIdle(vkDevice);

    for (FrameData& frame : frames) {
        vkDestroySemaphore(vkDevice, frame.imageAvailable, nullptr);
        vkDestroyFence(vkDevice, frame.computeFence, nullptr);
    }

    for (VkSemaphore semaphore : renderFinishedSemaphores) {
        vkDestroySemaphore(
            vkDevice,
            semaphore,
            nullptr
        );
    }

    swapchain.cleanUp();

    vkDestroyCommandPool(vkDevice, commandPool, nullptr);
    
    if (allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
    }

    vkDestroyDevice(vkDevice, nullptr);
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