#pragma once

#include <vulkan/vulkan.h>

#include "Core.h"
#include "Buffer.h"

namespace Vulkan {

class GraphicsDescriptorSet {
public:
    GraphicsDescriptorSet() = default;
    ~GraphicsDescriptorSet();

    void initialize(
        VulkanCore& core,
        Buffer& triangleObjectBuffer
    );

    void createDescriptorSetLayout();
    void createDescriptorSet(const Buffer& triangleObjectBuffer);

    VkDescriptorSet getDescriptorSet() const;
    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    VulkanCore* vulkanCore{ nullptr };
    
    // the triangle buffer is shared between the compute and 
    // graphics pipelines to avoid duplicating the same geometry.
    // the graphics descriptor set essentially points towards
    // the same triangle buffer as the compute descriptor set.
    VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
    VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
};

}