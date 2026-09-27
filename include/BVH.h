#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Geometry.h"

// axis aligned bounding box
struct AABB {
    glm::vec3 min;
    float pad0{};

    glm::vec3 max;
    float pad1{};
};

static_assert(sizeof(AABB) == 32);


// bounding volume heirarchy
struct alignas(16) BVHNode {
    glm::vec3 boundsMin;
    uint32_t leftChild{};

    glm::vec3 boundsMax;
    uint32_t rightChild{};

    uint32_t firstTriangle{};
    uint32_t triangleCount{};

    uint32_t pad0{};
    uint32_t pad1{};
};

static_assert(sizeof(BVHNode) == 48);


class BVH {
public:
    BVH() = default;

    void build(std::vector<Triangle>& triangles);

    const std::vector<BVHNode>& getNodes() const {
        return nodes;
    }

private:
    uint32_t buildNode(
        std::vector<Triangle>& triangles,
        uint32_t start,
        uint32_t cnt
    );

    AABB getTriangleBounds(const Triangle& triangle) const;

    // get triangle centroid for splitting
    glm::vec3 getTriangleCentroid(const Triangle& triangle) const;

    AABB combineBounds(
        const AABB& a,
        const AABB& b
    ) const;

    // compute bounds for a range of triangles
    AABB getBounds(
        const std::vector<Triangle>& triangles,
        uint32_t start,
        uint32_t end
    ) const;

private:
    std::vector<BVHNode>nodes;
    static constexpr uint32_t MAX_TRIANGLES_PER_LEAF = 4;
};