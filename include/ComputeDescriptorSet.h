#pragma once

#include "Core.h"
#include "Buffer.h"
#include <vulkan/vulkan_core.h>

namespace Vulkan {

class ComputeDescriptorSet {
public:
    ComputeDescriptorSet() = default;
    ~ComputeDescriptorSet();

    // TODO: group image views and buffers by structs
    void initialize(
        VulkanCore& vkCore, 
        VkImageView outputImageView, 
        VkImageView accumulatedImageView,
        const Buffer& sceneObjectBuffer,
        const Buffer& materialBuffer
    );
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet(
        VkImageView outputImageView, 
        VkImageView accumualtedImageView,
        const Buffer& sceneObjectBuffer,
        const Buffer& materialBuffer
    );
    void cleanup();
    
    VkDescriptorSet getDescriptorSet() const;
    VkDescriptorPool getDescriptorPool() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    VulkanCore* vulkanCore{ nullptr };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
};

}