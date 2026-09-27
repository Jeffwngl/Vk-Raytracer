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

bool hitAABB(
    Ray ray,
    vec3 boundsMin,
    vec3 boundsMax,
    float tMin,
    float tMax,
    out float tNear
) {
    tNear = tMin;

    for (int axis = 0; axis < 3; ++axis) {
        float invD = 1.0 / ray.direction[axis];
        float t0 = (boundsMin[axis] - ray.origin[axis]) * invD;
        float t1 = (boundsMax[axis] - ray.origin[axis]) * invD;

        if (invD < 0.0) {
            float temp = t0;
            t0 = t1;
            t1 = temp;
        }

        tNear = max(tNear, t0);
        tMax = min(tMax, t1);

        if (tMax <= tNear) {
            return false;
        }
    }

    return true;
}
