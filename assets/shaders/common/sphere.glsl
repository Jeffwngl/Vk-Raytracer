#ifndef SPHERE_GLSL
#define SPHERE_GLSL

struct Sphere {
    vec4 centerRadius;  // xyz = center, w = radius

    uint materialIndex; // index into materials[]
    uint pad0;
    uint pad1;
    uint pad2;
};


vec3 sphereCenter(Sphere sphere) {
    return sphere.centerRadius.xyz;
}


float sphereRadius(Sphere sphere) {
    return sphere.centerRadius.w;
}


bool hitSphere(
    Ray ray,
    Sphere sphere,
    float tMin,
    float tMax,
    out HitRecord rec
) {
    vec3 center = sphereCenter(sphere);
    float radius = sphereRadius(sphere);

    vec3 oc = center - ray.origin;

    float a = dot(ray.direction, ray.direction);

    float h = dot(ray.direction, oc);

    float c = dot(oc, oc) - radius * radius;

    float discriminant =  h * h - a * c;

    if (discriminant < 0.0) {
        return false;
    }

    float sqrtD = sqrt(discriminant);

    float root = (h - sqrtD) / a;

    if (root <= tMin || root >= tMax) {
        root = (h + sqrtD) / a;

        if (root <= tMin || root >= tMax) {
            return false;
        }
    }

    rec.t = root;
    rec.position = rayAt(ray, root);

    vec3 outwardNormal = (rec.position - center) / radius;

    setFaceNormal(
        ray,
        outwardNormal,
        rec
    );

    rec.materialIndex = sphere.materialIndex;

    return true;
}

#endif