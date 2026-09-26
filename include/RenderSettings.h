#pragma once

struct RenderSettings {
    uint32_t samplesPerPixel = 8;
    uint32_t maxBounces = 8;
    uint32_t accumulateRays = 1;
};