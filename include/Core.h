#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

#include "Queue.h"
#include "Swapchain.h"

namespace Vulkan {

struct FrameData {
    VkSemaphore imageAvailable{ VK_NULL_HANDLE };
    VkFence computeFence{ VK_NULL_HANDLE };
    VkCommandBuffer computeCommandBuffer{ VK_NULL_HANDLE };
};

class VulkanCore {

static constexpr uint32_t VulkanVersion{ VK_API_VERSION_1_3 };
static constexpr uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };

public:
    VulkanCore() = default;
    ~VulkanCore();

    bool running = false;

    bool initialize();

    VkDevice getDevice() const;
    VkPhysicalDevice getPhysicalDevice() const;
    VkSurfaceKHR getSurface() const;

    const Queue& getQueue() const;

    const Swapchain& getSwapchain() const;

    glm::vec2 getWindowSize() const;
    VmaAllocator getVmaAllocator() const;

    FrameData& getFrameData(uint32_t index);

    VkSemaphore getRenderFinishedSemaphore(
        uint32_t imageIndex
    ) const;

    VkSemaphore createSemaphore();
    VkFence createFence();

    void close();

private:
    void createWindow();
    void initializeInstance();
    void createSurface();
    void initializeDevice();
    void createLogicalDevice();
    void initializeVMA();
    void createSwapchain();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects(); 

    bool checkValidationLayerSupport();

    void setupDebugMessenger();
    void destroyDebugMessenger();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData
    );

    bool checkSuitableDevice(VkPhysicalDevice device);

    void cleanUp();

private:
    VkInstance instance{ VK_NULL_HANDLE };
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };

    Queue queue; // TODO: change to unique ptr
    uint32_t queueFamily{ 0 };

    SDL_Window* window{ nullptr };
    glm::ivec2 windowSize{};

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VmaAllocator allocator{ VK_NULL_HANDLE };

    Swapchain swapchain;

    VkCommandPool commandPool{ VK_NULL_HANDLE };
    std::vector<FrameData> frames{};

    std::vector<VkSemaphore> renderFinishedSemaphores;

    VkDebugUtilsMessengerEXT debugMessenger{ VK_NULL_HANDLE };

    const std::vector<const char*> validationLayers{
        "VK_LAYER_KHRONOS_validation"
    };
};

}