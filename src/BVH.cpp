#include "BVH.h"

#include <algorithm>
#include <iostream>

void BVH::build(std::vector<Triangle>& triangles) {
    nodes.clear();

    if (triangles.empty()) {
        return;
    }

    buildNode(triangles, 0, static_cast<uint32_t>(triangles.size()));
}

uint32_t BVH::buildNode (
    std::vector<Triangle>& triangles,
    uint32_t start,
    uint32_t cnt
) {
    uint32_t nodeIdx = static_cast<uint32_t>(nodes.size());

    nodes.push_back({});

    AABB bounds = getBounds(
        triangles,
        start,
        cnt
    );

    nodes[nodeIdx].boundsMin = bounds.min;
    nodes[nodeIdx].boundsMax = bounds.max;

    // stop recursing when leaf node has less than 4 triangles
    if (cnt <= MAX_TRIANGLES_PER_LEAF) {
        nodes[nodeIdx].firstTriangle = start;
        nodes[nodeIdx].triangleCount = cnt;

        return nodeIdx;
    }

    // find longest AABB axis to split down
    glm::vec3 extent = bounds.max - bounds.min;

    uint32_t axis = 0; // 0 = x, 1 = y, 2 = z

    if (extent.y > extent.x) {
        axis = 1;
    }
    
    if (extent.z > extent[axis]) {
        axis = 2;
    }

    std::sort(
        triangles.begin() + start,
        triangles.begin() + start + cnt,
        [this, axis](const Triangle& a,const Triangle& b) {
            return getTriangleCentroid(a)[axis] < getTriangleCentroid(b)[axis];
        }
    );

    uint32_t leftCnt = cnt / 2;
    uint32_t rightCnt = cnt - leftCnt;

    uint32_t leftChild = buildNode(
        triangles,
        start,
        leftCnt
    );

    uint32_t rightChild = buildNode(
        triangles,
        start + leftCnt,
        rightCnt
    );

    nodes[nodeIdx].leftChild = leftChild;
    nodes[nodeIdx].rightChild = rightChild;

    nodes[nodeIdx].triangleCount = 0;

    return nodeIdx;
}

AABB BVH::getTriangleBounds(const Triangle& triangle) const {
    glm::vec3 v0 = glm::vec3(triangle.v0.position);
    glm::vec3 v1 = glm::vec3(triangle.v1.position);
    glm::vec3 v2 = glm::vec3(triangle.v2.position);

    AABB bounds{};

    bounds.min = glm::min(v0, glm::min(v1, v2));
    bounds.max = glm::max(v0, glm::max(v1, v2));

    return bounds;
}

glm::vec3 BVH::getTriangleCentroid(const Triangle& triangle) const  { 
    glm::vec3 v0 = glm::vec3(triangle.v0.position);
    glm::vec3 v1 = glm::vec3(triangle.v1.position);
    glm::vec3 v2 = glm::vec3(triangle.v2.position);

    return (v0 + v1 + v2) / 3.0f;
}

AABB BVH::combineBounds(
    const AABB& a,
    const AABB& b
) const {
    AABB res{};

    res.min = glm::min(a.min, b.min);
    res.max = glm::max(a.max, b.max);

    return res;
}

AABB BVH::getBounds(
    const std::vector<Triangle>& triangles,
    uint32_t start,
    uint32_t end
) const {
    AABB bounds = getTriangleBounds(triangles[start]);

    for (uint32_t i = 1; i < end; ++i) {
        bounds = combineBounds(
            bounds,
            getTriangleBounds(triangles[start + i])
        );
    }

    return bounds;
}