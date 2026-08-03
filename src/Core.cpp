#include <cstddef>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <assert.h>

#include "Utils.h"

#include <stdexcept>
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>


export class VulkanCore {
public:
    bool initialize() {
        if (!std::filesystem::is_directory("assets")) {
            std::cerr << "Could not find assets folder from working director." << '\n';
            exit(-1);
        }

        

    }

private:
    void initializeWindow() {
        initializeInstance();

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
        std::cout << "Selected Device: " << deviceProperties.properties.deviceName << '\n';
    }

    void createLogicalDevice() {
        
    }

private:
    VkInstance instance{ VK_NULL_HANDLE };
};
