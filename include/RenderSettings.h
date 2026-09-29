#pragma once

enum class ViewMode : uint32_t {
    Raytrace = 0,
    Rasterized = 1,
    Wireframe = 2,
    BVHDebug = 3,
    BVHDepth = 4,
};

struct DebugSettings {
    ViewMode viewMode = ViewMode::Raytrace;
    uint32_t boxTestThreshold = 5;
};

struct RenderSettings {
    uint32_t samplesPerPixel = 1;
    uint32_t maxBounces = 2;
    uint32_t accumulateRays = 1;
};