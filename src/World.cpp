#include "World.h"

void World::Spheres() {
    uint32_t whiteMaterial = scene.addMaterial(whiteDiffuse);
    uint32_t silverMaterial = scene.addMaterial(silverMetal);
    uint32_t redMaterial = scene.addMaterial(redDiffuse);
    uint32_t glassMaterial = scene.addMaterial(glass);

    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, 0.0f, -3.0f, 1.0f),
        .materialIndex = silverMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(1.0f, 0.0f, -5.0f, 1.0f),
        .materialIndex = whiteMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(-1.4f, -0.5f, -2.0f, 0.5f),
        .materialIndex = glassMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(2.0f, -0.5f, -2.0f, 0.5f),
        .materialIndex = redMaterial,
    });
}