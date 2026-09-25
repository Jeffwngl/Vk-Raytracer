#pragma once

#include "Core.h"

#include <string>
#include <vulkan/vulkan_core.h>

#include "Camera.h"

namespace Vulkan {

// keep 16 byte intervals for GPU interpretation
struct alignas(16) PushConstants {
    // GPU data contains cameraPos, pixelStart, pixelDU/DV which matches GPU layout
    GPUData camera;

    // TODO: move below to independent struct
    uint32_t objectCnt;
    uint32_t samplesPerPixel = 8;
    uint32_t maxBounces = 8;
    uint32_t accumulatedFrames = 0;
    uint32_t accumulateRays = 1;
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