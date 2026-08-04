#include <cstddef>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <assert.h>

#include "SDL3/SDL_video.h"
#include "Utils.h"
#include "glm/glm.hpp"

#include <stdexcept>
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    std::vector<VkSurfaceFormatKHR>formats;
    std::vector<VkPresentModeKHR>presentModes;
};

export class VulkanCore {
public:
    bool initialize(uint32_t deviceNum) {
        if (!std::filesystem::is_directory("assets")) {
            std::cerr << "Could not find assets folder from working director." << '\n';
            exit(-1);
        }
        initializeInstance();
        initializeDevice(deviceNum);
        createLogicalDevice();
        createSurface();
        createSwapChain();

        
        return true;
    }

private:
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

    void initializeDevice(uint32_t deviceNum) {
        uint32_t deviceCnt{ 0 };
        utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, nullptr));

        if (deviceCnt == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");
        };

        std::vector<VkPhysicalDevice>devices(deviceCnt);
        utils::check(vkEnumeratePhysicalDevices(instance, &deviceCnt, devices.data()));

        // manually choose device
        assert(deviceNum < deviceCnt && deviceNum > 0);
        
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
        
        uint32_t selectedQueueFamily{ 0 };
        for (size_t i = 0; i < queueFamilyCnt; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { // vkCmdDraw
                selectedQueueFamily = i;
                break;
            }
        }
        utils::check(SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, selectedQueueFamily));

        const float queuePriority{ 1.0f };
        VkDeviceQueueCreateInfo queueCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = selectedQueueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        VkPhysicalDeviceFeatures deviceFeatures {
            // TODO: add features as needed
        };

        VkPhysicalDeviceVulkan12Features enabledVk12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
            // TODO: add features as needed
        };
        VkPhysicalDeviceVulkan13Features enabledVk13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
            // TODO: add features as needed
        };

        const std::vector<const char*>requiredDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo deviceCI{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCI,
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data()
            // TODO: .pNext and pEnabledFeatures to add
        };
        utils::check(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
    }

    void createSurface() {
        window = SDL_CreateWindow("Raytracer", 1280u, 720u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        assert(window);
        utils::check(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
        utils::check(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y));
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
        // check for wayland
        VkExtent2D swapchainExtent{ surfaceCaps.currentExtent };
        if (surfaceCaps.currentExtent.width == 0xFFFFFFFF) {
            swapchainExtent = {
                .width = static_cast<uint32_t>(windowSize.x),
                .height = static_cast<uint32_t>(windowSize.y)
            };
        }

        VkSwapchainCreateInfoKHR swapchainCI{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = surfaceCaps.minImageCount,
                .imageFormat = supportDetails.formats[swapFormat].format,
                .imageColorSpace = supportDetails.formats[swapFormat].colorSpace,
                .imageExtent{ .width = swapchainExtent.width, .height = swapchainExtent.height },
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        };
        utils::check(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));

        uint32_t imageCnt{ 0 };
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
            utils::check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCnt, details.presentModes.data()));
        }

        return details;
    }

private:
    VkInstance instance{ VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
    SDL_Window* window{};
    VkSurfaceKHR surface{ VK_NULL_HANDLE };
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    std::vector<VkImage>swapChainImages{};
    std::vector<VkImageView>swapChainImageViews{};

    glm::ivec2 windowSize{};

};
