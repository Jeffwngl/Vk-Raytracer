
#include <glm/glm.hpp>

struct GPUData {
    glm::vec4 position;
    glm::vec4 pixelStart;
    glm::vec4 pixelDU;
    glm::vec4 pixelDV;
};

class Camera {
public:
    Camera() = default;
    
    Camera(
        const glm::vec3& pos,
        const glm::vec3& target,
        float fov = 45.0f
    );

    void setPos(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void setFov(float fov);

    const glm::vec3 getPos() const;
    const glm::vec3 getTarget() const;
    float getFov() const;

    GPUData Camera::getGPUData(
        uint32_t width,
        uint32_t height
    ) const;

private:
    glm::vec3 pos{ 0.0f, 0.0f, 0.0f };
    glm::vec3 target{ 0.0f, 0.0f, -1.0f };
    glm::vec3 up{ 0.0f, 1.0f, 0.0f };
    float fov{ 45.0f };
};