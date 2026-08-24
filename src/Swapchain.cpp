#include "Swapchain.h"
#include "Core.h"
#include "Utils.h"

void Swapchain::initialize(
    VulkanCore& vulkanCore,
    VkExtent2D extent
) {
    this->vulkanCore = &vulkanCore;
    windowExtent = extent;

    createSwapchain();
    createImageViews();
}

void Swapchain::createSwapchain() {
    SwapchainSupportDetails supportDetails = 
        querySwapchainSupport(
            vulkanCore->getPhysicalDevice(), 
            vulkanCore->getSurface()
        );

    surfaceFormat = chooseSurfaceFormat(supportDetails.formats);
    presentMode = choosePresentMode(supportDetails.presentModes);
    extent = chooseSwapExtent(supportDetails.surfaceCapabilities);

    uint32_t imageCnt = supportDetails.surfaceCapabilities.minImageCount + 1;

    if (supportDetails.surfaceCapabilities.maxImageCount > 0 && imageCnt > supportDetails.surfaceCapabilities.maxImageCount) {
        imageCnt = supportDetails.surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vulkanCore->getSurface(),
        .minImageCount = imageCnt,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform = supportDetails.surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    utils::check(vkCreateSwapchainKHR(
        vulkanCore->getDevice(),
        &swapchainCI,
        nullptr,
        &swapchain
    ));

    utils::check(
        vkGetSwapchainImagesKHR(
            vulkanCore->getDevice(),
            swapchain,
            &imageCnt,
            nullptr
        )
    );

    swapchainImages.resize(imageCnt);

    utils::check(
        vkGetSwapchainImagesKHR(
            vulkanCore->getDevice(),
            swapchain,
            &imageCnt,
            swapchainImages.data()
        )
    );
}

void Swapchain::createImageViews() {
    swapchainImageViews.resize(
        swapchainImages.size()
    );

    for (size_t i = 0; i < swapchainImages.size(); ++i) {

        VkImageViewCreateInfo imageViewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,

            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        utils::check(
            vkCreateImageView(
                vulkanCore->getDevice(),
                &imageViewCI,
                nullptr,
                &swapchainImageViews[i]
            )
        );
    }
}

VkSwapchainKHR Swapchain::get() const {
    return this->swapchain;
}

VkFormat Swapchain::getFormat() const {
    return surfaceFormat.format;
}

const std::vector<VkImage>& Swapchain::getImages() const {
    return this->swapchainImages;
}

const std::vector<VkImageView>& Swapchain::getImageViews() const {
    return this->swapchainImageViews;
}

VkPresentModeKHR Swapchain::choosePresentMode(
    std::vector<VkPresentModeKHR>presentModes
) {
    for (size_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "Present mode: MAILBOX" << '\n';
            return presentModes[i];
        }
    }
    
    std::cout << "Present mode: FIFO" << '\n';
    return presentModes[0];
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(
    std::vector<VkSurfaceFormatKHR>surfaceFormats
) {
    for (size_t i = 0; i < surfaceFormats.size(); ++i) {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && 
            surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ) {
            return surfaceFormats[i];
        }
    }

    return surfaceFormats[0];
}


VkExtent2D Swapchain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& surfaceCapabilities
) {
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surfaceCapabilities.currentExtent;
    }
    else {
        VkExtent2D swapExtent = windowExtent;

        swapExtent.width = std::clamp(
            swapExtent.width,
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width
        );

        swapExtent.height = std::clamp(
            swapExtent.height,
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height
        );

        return swapExtent;
    }
}

SwapchainSupportDetails Swapchain::querySwapchainSupport(
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

void Swapchain::cleanUp() {
    if (!vulkanCore) {
        return;
    }

    VkDevice device = vulkanCore->getDevice();

    for (VkImageView view : swapchainImageViews) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, view, nullptr);
        }
    }

    swapchainImageViews.clear();
    swapchainImages.clear();

    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }

    vulkanCore = nullptr;
}

Swapchain::~Swapchain() {
    cleanUp();
}