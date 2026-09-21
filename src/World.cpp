#include "World.h"

void World::Spheres() {
    uint32_t whiteMaterial = scene.addMaterial(whiteDiffuse);
    uint32_t silverMaterial = scene.addMaterial(silverMetal);
    uint32_t redMaterial = scene.addMaterial(redDiffuse);

    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, 0.0f, -3.0f, 1.0f),
        .materialIndex = whiteMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(1.0f, 0.0f, -5.0f, 1.0f),
        .materialIndex = redMaterial,
    });
}