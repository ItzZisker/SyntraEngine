#pragma once

#include "Syngine/world/Coordination.hpp"

namespace syng
{
class Camera : public Coordination {
public:
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up);
    Camera(glm::vec3 position, float yaw, float pitch, glm::vec3 up);
    Camera(glm::vec3 position, glm::vec3 target);
    Camera(glm::vec3 position, float yaw, float pitch);

    void setDirection(const glm::vec3& direction) override;
    void setPosition(const glm::vec3& position) override;
    void setUp(const glm::vec3& up) override;

    glm::mat4 getViewMatrix();
    void updateViewMatrix();
private:
    glm::mat4 viewMatrix;
};
}