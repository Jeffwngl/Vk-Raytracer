#ifndef COLOR_GLSL
#define COLOR_GLSL

vec3 skyColor(vec3 direction) {
    float t = 0.5 * (normalize(direction).y + 1.0);

    return mix(vec3(1.0), vec3(0.2, 0.45, 1.0), t);
}


float linearToGamma(float x) {
    if (x > 0.0) {
        return sqrt(x);
    }

    return 0.0;
}

#endif