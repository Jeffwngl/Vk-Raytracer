#pragma once

#include "World.h"

#include <random>

namespace {

float randomFloat(float min = 0.0f, float max = 1.0f) {
    static std::mt19937 generator{ std::random_device{}() };
    std::uniform_real_distribution<float> distribution(min, max);

    return distribution(generator);
}

glm::vec3 randomVec3(float min = 0.0f, float max = 1.0f) {
    return {
        randomFloat(min, max),
        randomFloat(min, max),
        randomFloat(min, max)
    };
}

}