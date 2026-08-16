#include "Application.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>

bool Application::initialize() {
    vulkanCore.initialize();
    renderer.initialize(vulkanCore);
    return true;
}

void Application::run() {
    while (vulkanCore.running) {
        handleInput();
        handleDeltaTime();

        renderer.drawFrame();
    }

    // handle case do not draw if we are minimized
}

void Application::handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                vulkanCore.close();
                break;
            case SDL_EVENT_KEY_DOWN:
                // handle moving
                break;
            case SDL_EVENT_KEY_UP:
                // handle moving
                break;
            default:
                break;
        }
    }
}

void Application::handleDeltaTime() {
    lastTime = nowTime;
    nowTime = SDL_GetPerformanceCounter();
    deltaTime = (double)(nowTime - lastTime) / (double)SDL_GetPerformanceFrequency();
}

Application::~Application() {
    // 
}
