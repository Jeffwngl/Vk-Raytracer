#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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
    VulkanCore() = default;
    ~VulkanCore();

    bool initialize(uint32_t deviceNum);

private:
    void createWindow();
    void initializeInstance();
    void createSurface();
    void initializeDevice(uint32_t deviceNum);
    void createLogicalDevice();

    void initializeVMA();

    void createSwapChain();
    void createRenderPass();

    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    SwapchainSupportDetails querySwapChainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    );

    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& surfaceCaps
    );

    void cleanUp();

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };

    VkInstance instance{ VK_NULL_HANDLE };

    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkDevice device{ VK_NULL_HANDLE };

    VkQueue graphicsQueue{ VK_NULL_HANDLE };
    uint32_t queueFamily{ 0 };

    SDL_Window* window{ nullptr };
    glm::ivec2 windowSize{};

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    VkFormat swapChainFormat{ VK_FORMAT_UNDEFINED };

    std::vector<VkImage> swapChainImages{};
    std::vector<VkImageView> swapChainImageViews{};

    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VkCommandPool commandPool{ VK_NULL_HANDLE };

    std::vector<FrameData> frames{};
};
