#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

/**
 * Keeping the camera on the CPU is negligible as it only sends around 64-100 bytes of data
 * per frame compared to the shaders dispatching millions of shader invocations
 */
Camera::Camera(
    const glm::vec3& pos,
    const glm::vec3& target,
    float fov
) :
    pos{ pos },
    target{ target },
    fov{ fov } {}

void Camera::setPos(const glm::vec3& position) {
    this->pos = position;
}

void Camera::setTarget(const glm::vec3& target) {
    this->target = target;
}

void Camera::setFov(float fov) {
    this->fov = fov;
}

const glm::vec3 Camera::getPos() const {
    return this->pos;
}

const glm::vec3 Camera::getTarget() const {
    return this->target;
}

float Camera::getFov() const {
    return this->fov;
}

float& Camera::getFov() {
    return this->fov;
}

void Camera::moveForward(float amount) {
    glm::vec3 forward = glm::normalize(target - pos);
    glm::vec3 offset = forward * amount;

    pos += offset;
    target += offset;
}

void Camera::moveRight(float amount) {
    glm::vec3 forward = glm::normalize(target - pos);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 offset = right * amount;

    pos += offset;
    target += offset;
}

void Camera::moveUp(float amount) {
    glm::vec3 offset = up * amount;

    pos += offset;
    target += offset;
}


void Camera::yaw(float degrees) {
    glm::vec3 forward = glm::normalize(target - pos);
    glm::mat4 rotation = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(degrees) * -1.0f,
        up
    );
    glm::vec3 rotated = glm::vec3(rotation * glm::vec4(forward, 0.0f));

    target = pos + rotated;
}

void Camera::pitch(float degrees) {
    glm::vec3 forward = glm::normalize(target - pos);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::mat4 rotation = glm::rotate(
        glm::mat4(1.0f),
        glm::radians(degrees),
        right
    );
    glm::vec3 rotated = glm::vec3(rotation * glm::vec4(forward, 0.0f));

    target = pos + rotated;
}

/**
 * Takes in coordinates of camera position, target, up, fov and image width/height,
 * converts into the world space position of the first pixel, world space to move
 * one pixel to the right (u) and one down (v)
 * 
 * Moving these calculations to the GPU will result in the calculations run for every pixel
 * which would be less efficient as every pixel will compute the same information
 */
GPUData Camera::getGPUData(
    uint32_t width,
    uint32_t height
) const {
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    const float theta = glm::radians(fov);
    const float h = glm::tan(theta / 2.0f);

    const float focalLength = glm::length(pos - target);

    const float viewportHeight = 2.0f * h * focalLength;

    const float viewportWidth = viewportHeight * aspectRatio;

    // camera coordinate system
    glm::vec3 w = glm::normalize(pos - target);
    glm::vec3 u = glm::normalize(glm::cross(up, w));
    glm::vec3 v = glm::cross(w, u);

    glm::vec3 viewportU = viewportWidth * u;

    glm::vec3 viewportV = viewportHeight * -v;

    glm::vec3 pixelDeltaU = viewportU / static_cast<float>(width);

    glm::vec3 pixelDeltaV = viewportV / static_cast<float>(height);

    glm::vec3 viewportUpperLeft = pos - focalLength * w - viewportU / 2.0f - viewportV / 2.0f;

    glm::vec3 pixel00 = viewportUpperLeft + 0.5f * (pixelDeltaU + pixelDeltaV);

    return {
        glm::vec4(pos, 0.0f),
        glm::vec4(pixel00, 0.0f),
        glm::vec4(pixelDeltaU, 0.0f),
        glm::vec4(pixelDeltaV, 0.0f)
    };
}
