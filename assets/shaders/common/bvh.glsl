#ifndef BVH_GLSL
#define BVH_GLSL

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

float hitDistAABB(
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
            return FLOAT_MAX;
        }
    }

    return tNear;
}

bool hitBVH(
    Ray ray,
    float tMin,
    float tMax,
    out HitRecord closestHit,
    out uint boxTests
) {
    bool hitAnything = false;
    float closestT = tMax;

    boxTests = 0u;

    // BVH nodes are stored in a flat array
    // each node stores indexes like left and right
    // which are by itself indexes of the left and
    // right children.
    uint stack[MAX_DEPTH];
    uint stackPtr = 0;

    // test root first
    float rootNear;

    boxTests++;

    float rootDist = hitDistAABB(
        ray,
        nodes[0].boundsMin,
        nodes[0].boundsMax,
        tMin,
        closestT,
        rootNear
    );

    if (rootDist == FLOAT_MAX) {
        return false;
    }

    // iterative approach to BVH traversal to 
    // avoid using recursion on GPU.
    // push root onto stack.
    stack[stackPtr++] = 0;

    while (stackPtr > 0) {
        uint nodeIndex = stack[--stackPtr];

        BVHNode node = nodes[nodeIndex];

        // actual triangle intersection test is here once we have reached a
        // leaf node.
        if (node.triangleCount > 0) {
            for (uint i = 0; i < node.triangleCount; ++i) {
                uint triangleIndex = node.firstTriangle + i;

                HitRecord temp;

                if (hitTriangleMoller(
                    ray,
                    triangles[triangleIndex],
                    tMin,
                    closestT,
                    temp
                )) {
                    hitAnything = true;
                    closestT = temp.t;
                    closestHit = temp;
                }
            }
        }
        else {
            // else we traverse the internal nodes children
            // by pushing them onto the stack

            // do child ordering to avoid testing hidden AABBs
            uint childAIndex = node.leftChild;
            uint childBIndex = node.rightChild;
            BVHNode childA = nodes[childAIndex];
            BVHNode childB = nodes[childBIndex];

            float tNearA;
            float tNearB;

            // boxTests used to construct BVH heatmap.
            boxTests += 2u;


            float distA = hitDistAABB(
                ray,
                childA.boundsMin,
                childA.boundsMax,
                tMin,
                closestT,
                tNearA
            );
    
            float distB = hitDistAABB(
                ray,
                childB.boundsMin,
                childB.boundsMax,
                tMin,
                closestT,
                tNearB
            );

            // push further child first so nearer child
            // is processed first.
            if (distA > distB) {
                if (distA < FLOAT_MAX) stack[stackPtr++] = childAIndex;
                if (distB < FLOAT_MAX) stack[stackPtr++] = childBIndex;
            }
            else {
                if (distB < FLOAT_MAX) stack[stackPtr++] = childBIndex;
                if (distA < FLOAT_MAX) stack[stackPtr++] = childAIndex;
            }
        }
    }

    return hitAnything;
}

uint getBVHBoxTests(Ray ray) {
    HitRecord rec;
    uint boxTests;

    hitBVH(
        ray,
        0.001,
        FLOAT_MAX,
        rec,
        boxTests
    );

    return boxTests;
}

#endif