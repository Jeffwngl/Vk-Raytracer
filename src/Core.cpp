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
    }

    void initializeVMA() {

    }

    void createSwapChain() {


    }

private:
    VkInstance instance{ VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };
    SDL_Window* window{};
    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    glm::ivec2 windowSize{};

};
