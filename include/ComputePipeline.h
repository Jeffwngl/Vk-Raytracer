#pragma once

#include "Core.h"

#include <string>
#include <vulkan/vulkan_core.h>

namespace Vulkan {

struct PushConstants {
    uint32_t objectCnt;
};

class ComputePipeline {
public:
    ComputePipeline() = default;
    ~ComputePipeline();

    void initialize(VulkanCore& vkCore, std::string& path, VkDescriptorSetLayout descriptorSetLayout);

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;

private:
    VulkanCore* vulkanCore{ nullptr };
    VkPipeline pipeline{ VK_NULL_HANDLE };
    VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
};

}