#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class VulkanCore;

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain {
static constexpr uint32_t MAX_FRAMES_IN_FLIGHT{ 2 };
public:
    Swapchain() = default;
    ~Swapchain();

    void initialize(
        VulkanCore& vulkanCore,
        VkExtent2D extent
    );

    VkSwapchainKHR get() const;
    VkFormat getFormat() const;
    const std::vector<VkImageView>& getImageViews() const;
    const std::vector<VkImage>& getImages() const;

    void cleanUp();

private:
    void createSwapchain();
    void createImageViews();

    SwapchainSupportDetails querySwapchainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface
    );
    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& surfaceCaps
    );
    VkSurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>surfaceFormats
    );
    VkPresentModeKHR choosePresentMode(
        const std::vector<VkPresentModeKHR>presentMode
    );

private:
    VulkanCore* vulkanCore{ nullptr };
    VkExtent2D windowExtent;

    VkSurfaceFormatKHR surfaceFormat{};
    VkPresentModeKHR presentMode{};
    VkExtent2D extent;

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    std::vector<VkImage>swapchainImages{};
    std::vector<VkImageView>swapchainImageViews{};
};
