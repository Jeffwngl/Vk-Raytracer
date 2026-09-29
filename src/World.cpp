#include <iostream>

#include "World.h"
#include "Random.h"

void World::loadModel(std::string& path, const MaterialDefinition& materialDef, float scale, glm::vec3 pos, glm::vec3 rot) {

    std::cout << "Loading...\n";

    uint32_t material = scene.addMaterial(materialDef);

    uint32_t mesh = scene.addMesh(scene.loadObj(path));

    std::cout << "OBJ loaded\n";

    Object model {
        .meshIndex = mesh,
        .transform = {
            .position = pos,
            .rotation = rot,
            .scale = {scale, scale, scale},
        },
        .materialIndex = material,
    };

    scene.addObject(model);
}

void World::buildModel() {
    scene.buildTriangles();
    scene.buildBVH();

    std::cout
        << "BVH built with "
        << scene.getBVH().getNodes().size()
        << " nodes\n";

    std::cout
        << "Triangles built: "
        << scene.getTriangles().size()
        << '\n';
}

void World::Spheres() {
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

void World::TriangleTest() {
    uint32_t whiteMaterial = scene.addMaterial(whiteDiffuse);

    Mesh mesh;

    mesh.vertices = {
        {
            .position = {-1.0f, -0.75f, -3.0f, 1.0f},
            .normal   = { 0.0f,  0.0f,   1.0f, 0.0f},
            .uv       = {0.0f, 0.0f},
        },
        {
            .position = { 1.0f, -0.75f, -3.0f, 1.0f},
            .normal   = { 0.0f,  0.0f,   1.0f, 0.0f},
            .uv       = {1.0f, 0.0f},
        },
        {
            .position = { 0.0f,  1.0f,  -3.0f, 1.0f},
            .normal   = { 0.0f,  0.0f,   1.0f, 0.0f},
            .uv       = {0.5f, 1.0f},
        }
    };

    mesh.indices = {
        0, 1, 2
    };

    uint32_t meshIndex = scene.addMesh(std::move(mesh));

    Object triangle{
        .meshIndex = meshIndex,
        .materialIndex = whiteMaterial,
    };

    scene.addObject(triangle);
    scene.buildTriangles();

    std::cout
        << "Triangles: "
        << scene.getTriangles().size()
        << '\n';

    Camera& camera = scene.getCamera();

    camera.setPos({0.0f, 0.0f, 0.0f});
    camera.setTarget({0.0f, 0.0f, -3.0f});
    camera.setFov(45.0f);
}

void World::RayTracingInOneWeekend() {
    // ---------------------------------------------------------
    // Random helpers
    // ---------------------------------------------------------

    auto randomFloat = []() -> float {
        return static_cast<float>(rand()) /
               static_cast<float>(RAND_MAX);
    };

    auto randomFloatRange = [&](float min, float max) -> float {
        return min + (max - min) * randomFloat();
    };


    // ---------------------------------------------------------
    // Ground
    // ---------------------------------------------------------

    MaterialDefinition groundMaterial{
        .color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
        .params = glm::vec4(0.8f, 0.0f, 0.0f, 0.0f),
        .type = static_cast<uint32_t>(
            MaterialType::LAMBERTIAN
        ),
    };

    uint32_t groundMaterialIndex =
        scene.addMaterial(groundMaterial);

    scene.addSphere({
        .centerRadius =
            glm::vec4(0.0f, -1000.0f, 0.0f, 1000.0f),

        .materialIndex = groundMaterialIndex,
    });


    // ---------------------------------------------------------
    // Random spheres
    // ---------------------------------------------------------

    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {

            float chooseMaterial = randomFloat();

            glm::vec3 center{
                static_cast<float>(a) + 0.9f * randomFloat(),
                0.2f,
                static_cast<float>(b) + 0.9f * randomFloat()
            };

            // Don't spawn small spheres too close to
            // the large sphere at (4, 1, 0)
            if (
                glm::length(
                    center - glm::vec3(4.0f, 0.2f, 0.0f)
                ) <= 0.9f
            ) {
                continue;
            }


            // -------------------------------------------------
            // Diffuse
            // -------------------------------------------------

            if (chooseMaterial < 0.8f) {

                glm::vec3 randomColor1{
                    randomFloat(),
                    randomFloat(),
                    randomFloat()
                };

                glm::vec3 randomColor2{
                    randomFloat(),
                    randomFloat(),
                    randomFloat()
                };

                glm::vec3 albedo =
                    randomColor1 * randomColor2;

                MaterialDefinition material{
                    .color = glm::vec4(albedo, 1.0f),

                    .params = glm::vec4(0.8f, 0.0f, 0.0f, 0.0f),

                    .type = static_cast<uint32_t>(
                        MaterialType::LAMBERTIAN
                    ),
                };

                uint32_t materialIndex =
                    scene.addMaterial(material);

                scene.addSphere({
                    .centerRadius =
                        glm::vec4(center, 0.2f),

                    .materialIndex = materialIndex,
                });
            }


            // -------------------------------------------------
            // Metal
            // -------------------------------------------------

            else if (chooseMaterial < 0.95f) {

                glm::vec3 albedo{
                    randomFloatRange(0.5f, 1.0f),
                    randomFloatRange(0.5f, 1.0f),
                    randomFloatRange(0.5f, 1.0f)
                };

                float fuzz =
                    randomFloatRange(0.0f, 0.5f);

                MaterialDefinition material{
                    .color = glm::vec4(albedo, 1.0f),

                    // x = fuzz
                    .params =
                        glm::vec4(fuzz, 0.0f, 0.0f, 0.0f),

                    .type = static_cast<uint32_t>(
                        MaterialType::METAL
                    ),
                };

                uint32_t materialIndex =
                    scene.addMaterial(material);

                scene.addSphere({
                    .centerRadius =
                        glm::vec4(center, 0.2f),

                    .materialIndex = materialIndex,
                });
            }


            // -------------------------------------------------
            // Glass
            // -------------------------------------------------

            else {

                MaterialDefinition material{
                    .color =
                        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),

                    // x = index of refraction
                    .params =
                        glm::vec4(1.5f, 0.0f, 0.0f, 0.0f),

                    .type = static_cast<uint32_t>(
                        MaterialType::DIELECTRIC
                    ),
                };

                uint32_t materialIndex =
                    scene.addMaterial(material);

                scene.addSphere({
                    .centerRadius =
                        glm::vec4(center, 0.2f),

                    .materialIndex = materialIndex,
                });
            }
        }
    }


    // ---------------------------------------------------------
    // Main glass sphere
    // ---------------------------------------------------------

    MaterialDefinition material1{
        .color =
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),

        .params =
            glm::vec4(1.5f, 0.0f, 0.0f, 0.0f),

        .type = static_cast<uint32_t>(
            MaterialType::DIELECTRIC
        ),
    };

    uint32_t material1Index =
        scene.addMaterial(material1);

    scene.addSphere({
        .centerRadius =
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),

        .materialIndex = material1Index,
    });


    // ---------------------------------------------------------
    // Main diffuse sphere
    // ---------------------------------------------------------

    MaterialDefinition material2{
        .color =
            glm::vec4(0.4f, 0.2f, 0.1f, 1.0f),

        .params = glm::vec4(0.8f, 0.0f, 0.0f, 0.0f),

        .type = static_cast<uint32_t>(
            MaterialType::LAMBERTIAN
        ),
    };

    uint32_t material2Index =
        scene.addMaterial(material2);

    scene.addSphere({
        .centerRadius =
            glm::vec4(-4.0f, 1.0f, 0.0f, 1.0f),

        .materialIndex = material2Index,
    });


    // ---------------------------------------------------------
    // Main metal sphere
    // ---------------------------------------------------------

    MaterialDefinition material3{
        .color =
            glm::vec4(0.7f, 0.6f, 0.5f, 1.0f),

        .params =
            glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),

        .type = static_cast<uint32_t>(
            MaterialType::METAL
        ),
    };

    uint32_t material3Index =
        scene.addMaterial(material3);

    scene.addSphere({
        .centerRadius =
            glm::vec4(4.0f, 1.0f, 0.0f, 1.0f),

        .materialIndex = material3Index,
    });


    // ---------------------------------------------------------
    // Camera
    // ---------------------------------------------------------

    Camera& camera = scene.getCamera();

    camera.setPos(
        glm::vec3(13.0f, 2.0f, 3.0f)
    );

    camera.setTarget(
        glm::vec3(0.0f, 0.0f, 0.0f)
    );

    camera.setFov(20.0f);
}

void World::UtahTeapot() {
    Camera& camera = scene.getCamera();

    camera.setDefocusAngle(0.0f);

    camera.setPos(
        glm::vec3(0.0f, 3.0f, 20.0f)
    );

   camera.setTarget(
        glm::vec3(0.0f, 2.0f, 0.0f)
    );

    camera.setFov(20.0f);
    std::string path = "assets/models/teapot.obj";
    loadModel(path, whiteDiffuse, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));    
    buildModel();
}

void World::Suzanne() {
    Camera& camera = scene.getCamera();

    camera.setDefocusAngle(0.0f);

    camera.setPos(
        glm::vec3(-3.0f, 1.0f, 15.0f)
    );

    camera.setTarget(
        glm::vec3(-2.3f, 1.0f, 0.0f)
    );

    camera.setFov(20.0f);
    std::string path = "assets/models/suzanne.obj";
    loadModel(path, whiteDiffuse, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));    
    buildModel();
}

void World::Igea() {
    Camera& camera = scene.getCamera();

    camera.setDefocusAngle(0.0f);

    std::string path = "assets/models/igea.obj";
    loadModel(path, whiteDiffuse, 4.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    buildModel();
}

void World::Lucy() {
    Camera& camera = scene.getCamera();

    camera.setDefocusAngle(0.0f);

    camera.setPos(
        glm::vec3(0.0f, 0.0f, 0.0f)
    );

    camera.setTarget(
        glm::vec3(0.0f, 1.0f, -10.0f)
    );

    std::string path = "assets/models/lucy.obj";
    loadModel(path, whiteDiffuse, 5.0f, glm::vec3(1.0f, -0.8f, -8.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    buildModel();
}

void World::Lucies() {
    Camera& camera = scene.getCamera();

    camera.setDefocusAngle(0.0f);

    camera.setPos(
        glm::vec3(0.0f, 1.0f, 2.0f)
    );

    camera.setTarget(
        glm::vec3(0.0f, 0.5f, -8.0f)
    );

    std::string path = "assets/models/lucy.obj";
    loadModel(path, whiteDiffuse, 5.0f, glm::vec3(0.0f, -0.8f, -8.0f), glm::vec3(0.0f));

    loadModel(path, silverMetal, 5.0f, glm::vec3(2.0f, -0.8f, -11.0f), glm::vec3(0.0f));

    loadModel(path, glass, 5.0f, glm::vec3(4.0f, -0.8f, -14.0f), glm::vec3(0.0f));
    buildModel();
}