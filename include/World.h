#pragma once

#include "Scene.h"

class World {
public:
    World() = default;

    // default scene with spheres
    void Spheres();

    void TriangleTest();

    void RayTracingInOneWeekend();

    void UtahTeapot();

    void Suzanne();

    void Igea();

    Scene& getScene() {
        return scene;
    }

    const Scene& getScene() const {
        return scene;
    }

private:
    void loadModel(std::string& path, const MaterialDefinition& materialDef);

private:
    Scene scene{};
};