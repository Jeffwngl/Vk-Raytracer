#ifndef BVH_TYPES_GLSL
#define BVH_TYPES_GLSL

const uint MAX_DEPTH = 64;

struct BVHNode {
    vec3 boundsMin;
    uint leftChild;

    vec3 boundsMax;
    uint rightChild;

    uint firstTriangle;
    uint triangleCount;
    uint pad0;
    uint pad1;
};

#endif