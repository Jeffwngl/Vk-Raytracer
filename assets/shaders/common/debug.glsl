vec3 boxTestHeatmap(uint tests, uint threshold) {
    float t = clamp(
        float(tests) / max(float(threshold), 1.0),
        0.0,
        1.0
    );

    vec3 cold = vec3(0.0, 0.2, 1.0);
    vec3 mid  = vec3(0.0, 1.0, 0.2);
    vec3 hot  = vec3(1.0, 0.1, 0.0);

    if (t < 0.5) {
        return mix(cold, mid, t * 2.0);
    }

    return mix(mid, hot, (t - 0.5) * 2.0);
}