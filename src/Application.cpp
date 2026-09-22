#include "Application.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>

bool Application::initialize() {
    if (!vulkanCore.initialize()) {
        return false;
    };

    // initialize scene
    world.Spheres();

    if (!renderer.initialize(vulkanCore, world.getScene())) {
        return false;
    }

    imgui.initialize(vulkanCore);

    return true;
}

void Application::run() {
    while (vulkanCore.running) {

        Camera& camera = world.getScene().getCamera();

        handleInput(camera);

        if (!vulkanCore.running) {
            break;
        }

        handleDeltaTime();

        moveCamera();

        imgui.beginFrame();

        imgui.build(
            renderSettings,
            camera
        );

        renderer.drawFrame(imgui, renderSettings);
    }
}

void Application::handleInput(Camera& camera) {
    SDL_Event event;
    float sensitivity = 0.2f;

    while (SDL_PollEvent(&event)) {

        imgui.processEvent(event);

        switch (event.type) {
            case SDL_EVENT_QUIT:
                vulkanCore.close();

                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_W)
                    moveForward = true;

                if (event.key.key == SDLK_S)
                    moveBackward = true;

                if (event.key.key == SDLK_A)
                    moveLeft = true;

                if (event.key.key == SDLK_D)
                    moveRight = true;

                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_W)
                    moveForward = false;

                if (event.key.key == SDLK_S)
                    moveBackward = false;

                if (event.key.key == SDLK_A)
                    moveLeft = false;

                if (event.key.key == SDLK_D)
                    moveRight = false;

                break;

            case SDL_EVENT_MOUSE_MOTION:
                camera.yaw(
                    event.motion.xrel * sensitivity
                );

                camera.pitch(
                    -event.motion.yrel * sensitivity
                );

                break;

            default:

                break;
        }
    }
}

void Application::moveCamera() {
    float speed = 3.0f;

    Camera& camera = world.getScene().getCamera();

    if (moveForward) {
        camera.moveForward(
            speed * static_cast<float>(deltaTime)
        );
    }

    if (moveBackward) {
        camera.moveForward(
            -speed * static_cast<float>(deltaTime)
        );
    }

    if (moveRight) {
        camera.moveRight(
            speed * static_cast<float>(deltaTime)
        );
    }

    if (moveLeft) {
        camera.moveRight(
            -speed * static_cast<float>(deltaTime)
        );
    }
}

void Application::handleDeltaTime() {
    lastTime = nowTime;
    nowTime = SDL_GetPerformanceCounter();
    deltaTime = (double)(nowTime - lastTime) / (double)SDL_GetPerformanceFrequency();
    if (deltaTime > 0.0) {
        fps = 1.0 / deltaTime;
    }
}

Application::~Application() {
    vkDeviceWaitIdle(
        vulkanCore.getDevice().get()
    );

    imgui.cleanUp();
}