#pragma once

enum class ViewMode : uint32_t {
    Default = 0,
    Normals = 1,
    BVHDepth = 2,
    BoundingBoxes = 3,
};

struct RenderSettings {
    uint32_t samplesPerPixel = 1;
    uint32_t maxBounces = 2;
    uint32_t accumulateRays = 1;

    ViewMode viewMode = ViewMode::Default;
    uint32_t debugBVHNode = 0;
};