#ifndef TRIANGLE_GLSL
#define TRIANGLE_GLSL

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 uv;
    vec2 padding;
};

struct Triangle{
    Vertex v0;
    Vertex v1;
    Vertex v2;

    uint materialIndex;
    uint pad0;
    uint pad1;
    uint pad2;
};

/* 
Moller Trumbore algorithm for triangle intersection

the form of the point P laying on the triangle can be written as;

P = (1 - u - v)A + uB + vC
O + tD = A + u(B - A) + v(C - A)

then rearranging and substituting T = O - A, E1 = B - A, E2 = C - A;

[t]       1       [ | T   E1  E2 | ]
[u] = --------- * [ |-D   T   E2 | ]
[v]   |-D E1 E2|  [ |-D   E1  T  | ]

where A, B and C are the vertices of the triangle and u and v are the 
barycentric coordinate parameters.

the theorem solves a 3 equation linear system using Cramers rule.
*/
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle//moller-trumbore-ray-triangle-intersection.html
bool hitTriangleMoller(
    Ray ray,
    Triangle triangle,
    float tMin,
    float tMax,
    out HitRecord rec
) {
    float epsilon = 1e-8;

    vec3 edge0 = triangle.v1.position.xyz - triangle.v0.position.xyz;
    vec3 edge1 = triangle.v2.position.xyz - triangle.v0.position.xyz;

    vec3 pVec = cross(ray.direction, edge1);
    float determinant = dot(edge0, pVec);

    // ray is parallel to triangle
    if (abs(determinant) < epsilon) {
        return false;
    }

    float inverseDeterminant = 1.0 / determinant;

    vec3 tVec = ray.origin - triangle.v0.position.xyz;

    // barycentric coordinate u
    float u = dot(tVec, pVec) * inverseDeterminant;

    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 qVec = cross(tVec, edge0);

    // barycentric coordinate v
    float v = dot(ray.direction, qVec) * inverseDeterminant;

    if (v < 0.0 || u + v > 1.0) {
        return false;
    }

    // third barycentric coordinate
    float w = 1.0 - u - v;

    // distance along ray
    float t = dot(edge1, qVec) * inverseDeterminant;

    if (t <= tMin || t >= tMax) {
        return false;
    }

    rec.t = t;
    rec.position = rayAt(ray, t);
    rec.materialIndex = triangle.materialIndex;

    // interpolate OBJ vertex normals
    vec3 outwardNormal = normalize(
        w * triangle.v0.normal.xyz +
        u * triangle.v1.normal.xyz +
        v * triangle.v2.normal.xyz
    );

    setFaceNormal(
        ray,
        outwardNormal,
        rec
    );

    return true;
}

/*
// default bayecentric coordinate intersection
bool hitTriangleDefault(
    Ray ray,
    Triangle triangle,
    float tMin,
    float tMax,
    out HitRecord rec
) {
    float epsilon = 1e-8;

    vec3 edge0 = triangle.v1.position.xyz - triangle.v0.position.xyz;
    vec3 edge1 = triangle.v2.position.xyz - triangle.v0.position.xyz;

    vec3 triangleNormal = cross(edge0, edge1);
    // dot of triangle normal and ray direction calculates if the ray is parallel
    // or hitting the triangle at an angle
    float dotRayDir = dot(triangleNormal, ray.direction);

    // ray is parallel to triangle
    if (dotRayDir < epsilon) {
        return false;
    }

    float t = (dot(triangleNormal, v0) - dot(triangleNormal, ray.origin)) / dotRayDir;

    // behind camera
    if (t < 0) {
        return false;
    }

    // intersection point
    vec3 p = ray.origin + t * ray.direction;

    // float area = dot(triangleNormal, triangleNormal) / 2.0;

    // inside out test using barycentric coords
    vec3 c;

    // test edge v1 v2 (for triangle v1, v2, p)
    vec3 edge_v1_p = p - v1;
    vec3 edge_v1_v2 = v2 - v1;
    c = cross(edge_v1_v2, edge_v1_p);
    if (dot(triangleNormal, c)) {
        return false;
    }

    // test edge v2 v0 (for triangle v2, v0, p)
    vec3 edge_v2_p = p - v1;
    vec3 edge_v2_v0 = v0 - v2;
    c = cross(edge_v2_v0, edge_v2_p);
    if (dot(triangleNormal, c)) {
        return false;
    }

    // test edge v0 v1 (for triangle v0, v1, p)
    vec3 edge_v0_p = p - v0;
    vec3 edge_v0_v1 = edge0;
    c = cross(edge_v0_v1, edge_v0_p);
    if (dot(triangleNormal, c)) {
        return false;
    }

    return true;
};
*/

#endif