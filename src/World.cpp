#include "World.h"
#include "Random.h"

void World::Spheres() {
    uint32_t whiteMaterial = scene.addMaterial(whiteDiffuse);
    uint32_t silverMaterial = scene.addMaterial(silverMetal);
    uint32_t redMaterial = scene.addMaterial(redDiffuse);
    uint32_t glassMaterial = scene.addMaterial(glass);

    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, 0.0f, -3.0f, 1.0f), // x, y, z, r
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

void World::RayTracingInOneWeekend() {
    uint32_t whiteMaterial  = scene.addMaterial(whiteDiffuse);
    uint32_t silverMaterial = scene.addMaterial(silverMetal);
    uint32_t redMaterial    = scene.addMaterial(redDiffuse);
    uint32_t glassMaterial  = scene.addMaterial(glass);

    scene.getCamera().setPos(glm::vec3(13.0, 2.0, 3.0));
    scene.getCamera().setTarget(glm::vec3(0.0, 0.0, 0.0));
    scene.getCamera().setFov(20.0);

    // Ground
    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, -1000.0f, 0.0f, 1000.0f),
        .materialIndex = whiteMaterial,
    });

    // Main glass sphere
    scene.addSphere({
        .centerRadius = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        .materialIndex = glassMaterial,
    });

    // Main diffuse sphere
    scene.addSphere({
        .centerRadius = glm::vec4(-4.0f, 1.0f, 0.0f, 1.0f),
        .materialIndex = redMaterial,
    });

    // Main metal sphere
    scene.addSphere({
        .centerRadius = glm::vec4(4.0f, 1.0f, 0.0f, 1.0f),
        .materialIndex = silverMaterial,
    });

    // Small spheres
    scene.addSphere({
        .centerRadius = glm::vec4(-2.5f, 0.2f, 1.5f, 0.2f),
        .materialIndex = whiteMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(-1.5f, 0.2f, -1.0f, 0.2f),
        .materialIndex = silverMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(-0.5f, 0.2f, 2.0f, 0.2f),
        .materialIndex = redMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(1.5f, 0.2f, -1.5f, 0.2f),
        .materialIndex = glassMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(2.5f, 0.2f, 1.5f, 0.2f),
        .materialIndex = whiteMaterial,
    });

    scene.addSphere({
        .centerRadius = glm::vec4(3.0f, 0.2f, -2.0f, 0.2f),
        .materialIndex = silverMaterial,
    });
}