#pragma once

#include "Core.h"

class ImGuiLayer {
public:
    void initialize(Vulkan::VulkanCore& core);
    void processEvent(const SDL_Event& event);
    void beginFrame();
    void build(float fps);
    void render(VkCommandBuffer commandBuffer);
    void cleanUp();

private:
    Vulkan::VulkanCore* vulkanCore{ nullptr };
};