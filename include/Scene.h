#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>
#include <tiny_obj_loader.h>

#include "Camera.h"
#include "Geometry.h"
#include "BVH.h"


// TODO: move materials to separate file
enum class MaterialType : uint32_t {
    LAMBERTIAN = 0,
    METAL = 1,
    DIELECTRIC = 2,
};

/**
 * using vec4 makes CPU/GPU alignment much easier.
 * using an abstract class as opposed to a plain struct
 * would make more sense here, however, everything
 * would still need to be flattened later anyways.
 * padding is used to fill up the rest of the space
 * so the GPU uses the same 48 byte layout.
 */
struct MaterialDefinition {
    glm::vec4 color; // r, g, b, a 16 bytes
    glm::vec4 params; // x, y, z, w 16 bytes
    uint32_t type; // 4 bytes
    uint32_t padding[3]{}; // 12 bytes
};

static_assert(sizeof(MaterialDefinition) == 48);

inline const MaterialDefinition whiteDiffuse {
    .color = glm::vec4{1.0f},
    .params = glm::vec4{0.6f, 0.0f, 0.0f, 0.0f}, // (x is used for absorption in lambertian)
    .type = static_cast<uint32_t>(MaterialType::LAMBERTIAN),
};

inline const MaterialDefinition redDiffuse {
    .color = glm::vec4{0.8f, 0.1f, 0.1f, 1.0f},
    .params = glm::vec4{0.5f, 0.0f, 0.0f, 0.0f},
    .type = static_cast<uint32_t>(MaterialType::LAMBERTIAN),
};

inline const MaterialDefinition silverMetal {
    .color = glm::vec4{0.8f, 0.8f, 0.8f, 1.0f},
    .params = glm::vec4{0.05f, 0.0f, 0.0f, 0.0f}, // (x is used for fuzz in metal)
    .type = static_cast<uint32_t>(MaterialType::METAL),
};

inline const MaterialDefinition glass {
    .color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
    .params = glm::vec4{1.52f, 0.0f, 0.0f, 0.0f}, // (x is used for refractivity)
    .type = static_cast<uint32_t>(MaterialType::DIELECTRIC),
};


struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};


struct Transform {
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f };
};


struct Object {
    uint32_t meshIndex{ 0 };
    Transform transform;
    uint materialIndex{ 0 };
};


class Scene {
public:
    void addSphere(const Sphere& sphere);

    void addObject(const Object& object);

    uint32_t addMesh(Mesh mesh);

    uint32_t addMaterial(const MaterialDefinition& material);

    Mesh loadObj(const std::string& path);

    void buildTriangles();

    void buildBVH();

    const std::vector<Sphere>& getSpheres() const {
        return spheres;
    }

    const std::vector<Triangle>& getTriangles() const {
        return triangles;
    }

    const std::vector<Mesh>& getMeshes() const {
        return meshes;
    }

    const std::vector<MaterialDefinition>& getMaterials() const {
        return materials;
    }

    const Camera& getCamera() const {
        return camera;
    }

    Camera& getCamera() {
        return camera;
    }

    const BVH& getBVH() const {
        return bvh;
    }

    bool isDirty() const {
        return dirty;
    }

    void clearDirty() {
        dirty = false;
    }

private:
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;
    std::vector<Mesh> meshes;
    std::vector<Object> objects;
    std::vector<MaterialDefinition> materials;

    BVH bvh;

    Camera camera;

    bool dirty{true};
};