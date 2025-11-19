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

    void setPosition(const glm::vec3& position);
    void setDirection(const glm::vec3& direction, const glm::vec3 upHint = glm::vec3(0,1,0));

    glm::vec3 getPosition();
    glm::vec3 getDirection();
    glm::vec3 getUp();

    glm::mat4 getTransform();
    glm::mat4 getViewMatrix();
    void updateViewMatrix();
private:
    Coordination coords;
    glm::mat4 viewMatrix;
};

}