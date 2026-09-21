#ifndef RAY_GLSL
#define RAY_GLSL

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitRecord {
    vec3 position;
    vec3 normal;
    float t;
    bool frontFace;
    uint materialIndex;
};


vec3 rayAt(Ray ray, float t) {
    return ray.origin + t * ray.direction;
}


void setFaceNormal(
    Ray ray,
    vec3 outwardNormal,
    inout HitRecord rec
) {
    rec.frontFace = dot(ray.direction, outwardNormal) < 0.0;

    rec.normal = rec.frontFace ? outwardNormal : -outwardNormal;
}

#endif