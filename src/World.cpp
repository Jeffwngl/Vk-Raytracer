#include "World.h"

void World::Spheres() {
    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, 0.0f, -3.0f, 1.0f),
        .color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
    });

    scene.addSphere({
        .centerRadius = glm::vec4(1.0f, 0.0f, -4.0f, 0.8f),
        .color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
    });
}