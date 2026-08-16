#pragma once

#include "Core.h"

#include <string>
#include <vulkan/vulkan_core.h>

class ComputePipeline {
public:
    ComputePipeline() = default;
    ~ComputePipeline();

    void initialize(VulkanCore& vkCore, std::string& path, VkDescriptorSetLayout descriptorSetLayout);

    VkPipeline getPipeline();
    VkPipelineLayout getPipelineLayout();

private:
    VulkanCore* vulkanCore;
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
};
