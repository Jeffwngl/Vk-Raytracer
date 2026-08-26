#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>

// const double pi = 3.1415926535897932385;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
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
    Mesh* mesh{ nullptr };
    Transform transform;
    uint32_t materialIndex{ 0 };
};


struct Camera {
    glm::vec3 position{ 0.0f };
};


struct Sphere {
    glm::vec4 centerRadius; // (x, y, z, r) 16 bytes
    glm::vec4 color; // (r, g, b, a) 16 bytes

    glm::vec3 center() const {
        return glm::vec3(centerRadius);
    }

    float radius() const {
        return centerRadius.w;
    }
};


class Scene {
public:
    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
        dirty = true;
    }

    const std::vector<Sphere>& getObjects() const {
        return spheres; // TODO: change to a general type later
    }

    bool isDirty() const {
        return dirty;
    }

    void clearDirty() {
        dirty = false;
    }

private:
    std::vector<Sphere> spheres;

    bool dirty{true};
};