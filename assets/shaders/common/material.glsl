#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

const uint MATERIAL_LAMBERTIAN = 0;
const uint MATERIAL_METAL = 1;
const uint MATERIAL_DIELECTRIC = 2;

const uint MATERIAL_GROUND = MATERIAL_LAMBERTIAN;
const float AIR_REFRACTIVITY = 1.0;


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

float reflectance(
    float cosTheta,
    float refractiveIndex
) {
    // use schlick approximation for reflectance
    float r0 = (1.0 - refractiveIndex) / (1.0 + refractiveIndex);
    r0 = r0 * r0;
    
    return r0 + (1.0 - r0) * pow((1 - cosTheta), 5);
}

vec3 refract(
    Ray ray,
    vec3 normal,
    float refractivityIn,
    float refractivityOut,
    inout uint rngState
) {
    vec3 unitDir = normalize(ray.direction);
    float cosTheta = min(dot(-unitDir, normal), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float eta = refractivityIn / refractivityOut;

    float randomValue = randomFloat(rngState);
    
    if (
        eta * sinTheta > 1.0 ||
        reflectance(cosTheta, refractivityIn) > randomValue
    ) {
        return reflect(ray, normal);
    }

    vec3 rayOutPerp = eta * (unitDir + cosTheta * normal);
    vec3 rayOutParallel = -sqrt(abs(1.0 - dot(rayOutPerp, rayOutPerp))) * normal;

    return rayOutPerp + rayOutParallel;
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

        // material.params.x here is reflectivity
        attenuation = material.color.rgb * material.params.x;

        return true;
    }

    else if (material.type == MATERIAL_METAL) {
        vec3 reflectedDirection = reflect(ray, rec.normal);

        scattered.origin = rec.position + rec.normal * 0.0001;

        // material.params.x here is fuzziness of the radius of the fuzz
        // factor unit sphere
        scattered.direction = 
            normalize(reflectedDirection) + 
            (material.params.x * generateRandomUnitVector(rngState));

        attenuation = material.color.rgb;

        return (dot(scattered.direction, rec.normal) > 0);
    }

    else if (material.type == MATERIAL_DIELECTRIC) {
        vec3 refractedDirection = refract(
            ray,
            rec.normal,
            AIR_REFRACTIVITY,
            material.params.x, // materials.params.x is the refractivity of the object
            rngState
        );

        scattered.origin = rec.position + rec.normal * 0.0001;

        scattered.direction = normalize(refractedDirection);

        attenuation = material.color.rgb;

        return true;
    }

    return false;
}

#endif