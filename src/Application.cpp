#include "Application.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>

enum class SceneName {
    Spheres,
    OneWeekend,
    Igea,
    Lucy,
    Teapot,
    Suzanne,
    Lucies
};

const std::unordered_map<std::string, SceneName> scenes = {
    {"spheres", SceneName::Spheres},
    {"oneWeekend", SceneName::OneWeekend},
    {"igea", SceneName::Igea},
    {"lucy", SceneName::Lucy},
    {"teapot", SceneName::Teapot},
    {"suzanne", SceneName::Suzanne},
    {"lucies", SceneName::Lucies},
    {"", SceneName::Spheres}
};

bool Application::initialize(std::string& modelName) {
    if (!vulkanCore.initialize()) {
        return false;
    };

    // initialize scene
    auto it = scenes.find(modelName);

    if (it == scenes.end()) {
        std::cerr << "Unknown model: " << modelName << '\n';
        return false;
    }

    SceneName model = it->second;

    switch (model) {
        case SceneName::Spheres:
            world.Spheres();
            break;

        case SceneName::OneWeekend:
            world.RayTracingInOneWeekend();
            break;

        case SceneName::Igea:
            world.Igea();
            break;

        case SceneName::Lucy:
            world.Lucy();
            break;

        case SceneName::Teapot:
            world.UtahTeapot();
            break;

        case SceneName::Suzanne:
            world.Suzanne();
            break;
        
        case SceneName::Lucies:
            world.Lucies();
            break;

        default:
            world.Spheres();
            break;
    }

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

        // imgui.build() returns true if variables have been modified
        if (imgui.build(
            renderSettings,
            debugSettings,
            camera,
            renderer.getAccumulatedFrames(),
            world.getScene()
        )) {
            renderer.resetAccumulatedFrames();
        }

        renderer.drawFrame(imgui, renderSettings, debugSettings);

        if (renderSettings.accumulateRays == 1u && debugSettings.viewMode == ViewMode::Raytrace) {
            renderer.advanceAccumulatedFrames();
        }
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
                if (event.key.key == SDLK_T && !event.key.repeat)
                    cameraMouseEnabled = !cameraMouseEnabled;

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
                if (cameraMouseEnabled) {
                    renderer.resetAccumulatedFrames();

                    camera.yaw(
                        event.motion.xrel * sensitivity
                    );

                    camera.pitch(
                        -event.motion.yrel * sensitivity
                    );
                }

                break;

            default:

                break;
        }
    }
}

void Application::moveCamera() {
    Camera& camera = world.getScene().getCamera();
    float speed = camera.getSpeed();

    bool cameraMoved = false;

    if (moveForward) {
        camera.moveForward(
            speed * static_cast<float>(deltaTime)
        );
        cameraMoved = true;
    }

    if (moveBackward) {
        camera.moveForward(
            -speed * static_cast<float>(deltaTime)
        );
        cameraMoved = true;
    }

    if (moveRight) {
        camera.moveRight(
            speed * static_cast<float>(deltaTime)
        );
        cameraMoved = true;
    }

    if (moveLeft) {
        camera.moveRight(
            -speed * static_cast<float>(deltaTime)
        );
        cameraMoved = true;
    }

    if (cameraMoved) {
        renderer.resetAccumulatedFrames();
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