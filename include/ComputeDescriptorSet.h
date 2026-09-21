#pragma once

#include "Core.h"
#include "Buffer.h"
#include <vulkan/vulkan_core.h>

namespace Vulkan {

class ComputeDescriptorSet {
public:
    ComputeDescriptorSet() = default;
    ~ComputeDescriptorSet();

    void initialize(
        VulkanCore& vkCore, 
        VkImageView outputImageView, 
        const Buffer& sceneObjectBuffer,
        const Buffer& materialBuffer
    );
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet(
        VkImageView outputImageView, 
        const Buffer& sceneObjectBuffer,
        const Buffer& materialBuffer
    );
    void cleanup();
    
    VkDescriptorSet getDescriptorSet() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    VulkanCore* vulkanCore{ nullptr };

    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
    VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
};

}