#pragma once

#include <glm/glm.hpp>
#include <cstdint>

struct Vertex {
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec2 uv;
    glm::vec2 padding;
};

static_assert(sizeof(Vertex) == 48);


struct alignas(16) Triangle {
    Vertex v0;
    Vertex v1;
    Vertex v2;

    uint32_t materialIndex{ 0 };
    uint32_t padding[3]{};
};

static_assert(sizeof(Triangle) == 160);

struct Sphere {
    glm::vec4 centerRadius; // (x, y, z, r) 16 bytes
    uint32_t materialIndex{ 0 }; // 4 bytes
    uint32_t padding[3]{}; // 12 bytes

    glm::vec3 center() const {
        return glm::vec3(centerRadius);
    }

    float radius() const {
        return centerRadius.w;
    }
};

static_assert(sizeof(Sphere) == 32);