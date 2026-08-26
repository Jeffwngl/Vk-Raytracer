#pragma once

#include <SDL3/SDL.h>

#include "Core.h"
#include "Renderer.h"
#include "World.h"

class Application {
public:
    Application() = default;
    ~Application() = default;

    bool initialize();

    void run();

private:
    void handleInput();
    void handleDeltaTime();

private:
    Vulkan::VulkanCore vulkanCore;
    Renderer renderer;
    World world;
    Uint64 nowTime = SDL_GetPerformanceCounter();
    Uint64 lastTime = 0;
    double deltaTime = 0;
};
