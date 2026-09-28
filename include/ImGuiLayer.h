#pragma once

#include "Core.h"
#include "Camera.h"
#include "RenderSettings.h"
#include "Scene.h"

class ImGuiLayer {
public:
    void initialize(Vulkan::VulkanCore& core);
    void processEvent(const SDL_Event& event);
    void beginFrame();
    bool build(
        RenderSettings& settings, 
        DebugSettings& debug,
        Camera& camera, 
        uint32_t accumulatedFrames,
        Scene scene
    );
    void render(VkCommandBuffer commandBuffer);
    void cleanUp();

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
    int currMode;
    // used only for SliderScalar
    uint32_t sliderMin = 1;
    uint32_t sliderMax = 64;
};