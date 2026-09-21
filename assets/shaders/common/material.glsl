#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

const uint MATERIAL_LAMBERTIAN = 0;
const uint MATERIAL_METAL = 1;

const uint MATERIAL_GROUND = MATERIAL_LAMBERTIAN;


struct Material {
    vec4 color;
    vec4 params;

    uint type;
    uint pad0;
    uint pad1;
    uint pad2;
};

vec3 reflect(
    Ray ray,
    vec3 normal
) {
    return ray.direction - 2.0 * dot(ray.direction, normal) * normal;
}


bool scatter(
    Ray ray,
    Material material,
    HitRecord rec,
    inout uint rngState,
    out Ray scattered,
    out vec3 attenuation
) {
    if (material.type == MATERIAL_LAMBERTIAN) {
        vec3 scatterDirection = rec.normal + generateRandomUnitVector(rngState);

        if (nearZero(scatterDirection)) {
            scatterDirection = rec.normal;
        }

        scattered.origin = rec.position + rec.normal * 0.0001;

        scattered.direction = normalize(scatterDirection);

        attenuation = material.color.rgb * material.params.x;

        return true;
    }

    else if (material.type == MATERIAL_METAL) {
        vec3 reflectedDirection = reflect(ray, rec.normal);

        scattered.origin = rec.position + rec.normal * 0.0001;

        scattered.direction = normalize(reflectedDirection);

        attenuation = material.color.rgb;

        return true;
    }

    return false;
}

#endif