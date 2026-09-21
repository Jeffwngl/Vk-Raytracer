#ifndef SAMPLING_GLSL
#define SAMPLING_GLSL

bool nearZero(vec3 v) {
    const float epsilon = 1e-8;

    return
        abs(v.x) < epsilon &&
        abs(v.y) < epsilon &&
        abs(v.z) < epsilon;
}

vec3 generateRandomInUnitSphere(inout uint rngState) {
    const float epsilon = 1e-8;

    while (true) {
        vec3 p = vec3(
            rnd(rngState),
            rnd(rngState),
            rnd(rngState)
        ) * 2.0 - 1.0;

        float lensq = dot(p, p);

        if (epsilon < lensq && lensq <= 1.0) {
            return p;
        }
    }
}

vec3 generateRandomUnitVector(inout uint rngState) {
    return normalize(
        generateRandomInUnitSphere(rngState)
    );
}

#endif