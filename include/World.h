#pragma once

#include "Scene.h"

class World {
public:
    World() = default;

    // scene names
    void Spheres();
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