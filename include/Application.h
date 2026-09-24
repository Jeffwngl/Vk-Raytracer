#pragma once

#include <SDL3/SDL.h>

#include "Core.h"
#include "Renderer.h"
#include "World.h"
#include "ImGuiLayer.h"
#include "RenderSettings.h"

class Application {
public:
    Application() = default;
    ~Application();

    bool initialize();

    void run();

private:
    void handleInput(Camera& camera);
    void handleDeltaTime();
    void moveCamera();

private:
    Vulkan::VulkanCore vulkanCore;
    Renderer renderer;
    World world;
    Uint64 nowTime = SDL_GetPerformanceCounter();
    Uint64 lastTime = 0;
    double deltaTime = 0;
    double fps = 0;
    ImGuiLayer imgui;
    bool moveForward{ false };
    bool moveBackward{ false };
    bool moveLeft{ false };
    bool moveRight{ false };
    bool cameraMouseEnabled{ false };

    RenderSettings renderSettings{};
};