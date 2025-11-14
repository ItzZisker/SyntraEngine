#pragma once

#include "Syngine/world/Coordination.hpp"

namespace syng
{

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up);
    Camera(glm::vec3 position, float yaw, float pitch, glm::vec3 up);
    Camera(glm::vec3 position, glm::vec3 target);
    Camera(glm::vec3 position, float yaw, float pitch);

    void setYaw(float degrees);
    void setPitch(float degrees);
    void setRoll(float degrees);

    void setPosition(const glm::vec3& position);
    void setUp(const glm::vec3& up);

    glm::vec3 getPosition();
    glm::vec3 getDirection();
    glm::vec3 getUp();

    float getYaw();
    float getPitch();
    float getRoll();

    glm::mat4 getTransform();
    glm::mat4 getViewMatrix();
    void updateViewMatrix();
private:
    Coordination coords;
    glm::mat4 viewMatrix;
};

}