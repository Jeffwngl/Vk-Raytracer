#pragma once

#include "Core.h"
#include "Renderer.h"
#include <SDL3/SDL.h>

class Application {
public:
    Application() = default;
    ~Application();

    bool initialize();

    void run();

private:
    void handleInput();
    void handleDeltaTime();

private:
    VulkanCore vulkanCore;
    Renderer renderer;
    Uint64 nowTime = SDL_GetPerformanceCounter();
    Uint64 lastTime = 0;
    double deltaTime = 0;
};
