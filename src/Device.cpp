#include "Device.h"

#include <vector>
#include <SDL3/SDL_vulkan.h>

#include "Utils.h"

namespace Vulkan {

VkPhysicalDevice Device::getPhysicalDevice() const {
    return physicalDevice;
};
VkDevice Device::get() const {
    return device;
};
uint32_t Device::getQueueFamily() const {
    return queueFamily;
};
const Queue& Device::getQueue() const {
    return queue;
};

bool Device::initialize(
    VkInstance instance,
    VkSurfaceKHR surface
) {
    selectPhysicalDevice(instance);
    createLogicalDevice(instance);

    return true;
}

void Device::selectPhysicalDevice(
    VkInstance instance
) {
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

bool Device::checkSuitableDevice(
    VkPhysicalDevice device
) {
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

void Device::createLogicalDevice(
    VkInstance instance
) {
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

}