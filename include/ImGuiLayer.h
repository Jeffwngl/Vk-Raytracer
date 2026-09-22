#pragma once

#include "Core.h"
#include "Camera.h"
#include "RenderSettings.h"

class ImGuiLayer {
public:
    void initialize(Vulkan::VulkanCore& core);
    void processEvent(const SDL_Event& event);
    void beginFrame();
    void build(RenderSettings& settings, Camera& camera);
    void render(VkCommandBuffer commandBuffer);
    void cleanUp();

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
};