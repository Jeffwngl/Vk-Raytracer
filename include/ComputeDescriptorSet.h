#pragma once

#include "Core.h"
#include <vulkan/vulkan_core.h>

class ComputeDescriptorSet {
public:
    ComputeDescriptorSet() = default;
    ~ComputeDescriptorSet();

    void initialize(VulkanCore& vkCore, VkImageView outputImageView);
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet(VkImageView outputImageView);
    void cleanup();
    
    VkDescriptorSet getDescriptorSet();
    VkDescriptorSetLayout getDescriptorSetLayout();

private:
    VulkanCore* vulkanCore{ nullptr };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
};
