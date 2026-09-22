#pragma once

#include "Scene.h"

class World {
public:
    World() = default;

    // default scene with spheres
    void Spheres();

    void RayTracingInOneWeekend();

    // to implement
    void CornellBox();

    Scene& getScene() {
        return scene;
    }

    const Scene& getScene() const {
        return scene;
    }

private:
    Scene scene{};
};