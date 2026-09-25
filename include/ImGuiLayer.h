#pragma once

#include "Core.h"
#include "Camera.h"
#include "RenderSettings.h"

class ImGuiLayer {
public:
    void initialize(Vulkan::VulkanCore& core);
    void processEvent(const SDL_Event& event);
    void beginFrame();
    bool build(
        RenderSettings& settings, 
        Camera& camera, 
        uint32_t accumulatedFrames
    );
    void render(VkCommandBuffer commandBuffer);
    void cleanUp();

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
    // used only for SliderScalar
    uint32_t sliderMin = 0;
    uint32_t sliderMax = 64;
};