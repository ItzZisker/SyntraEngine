#include "Camera.hpp"
#include "Syngine/world/WorldObject.hpp"

#include <glm/ext/matrix_transform.hpp>

using namespace syng;

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    glm::vec3 dir = glm::normalize(target - position);
    setDirection(dir);
    setUp(up);
    setPosition(position);
    updateViewMatrix();
}

Camera::Camera(glm::vec3 position, float yaw, float pitch, glm::vec3 up)
    : Camera(position, glm::vec3(
                 cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                 sin(glm::radians(pitch)),
                 sin(glm::radians(yaw)) * cos(glm::radians(pitch)))) {
    setUp(up);
}

Camera::Camera(glm::vec3 position, glm::vec3 target)
    : Camera(position, target, glm::vec3(0, 1, 0)) {}

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : Camera(position, yaw, pitch, glm::vec3(0, 1, 0)) {}

void Camera::updateViewMatrix() {
    glm::vec3 pos = getPosition();
    viewMatrix = glm::lookAt(pos, pos + getDirection(), getUp());
}

glm::mat4 Camera::getViewMatrix() {
    return viewMatrix;
}

void Camera::setUp(const glm::vec3& newUp) {
    Coordination::setUp(newUp);
    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3& position) {
    Coordination::setPosition(position);
    updateViewMatrix();
}

void Camera::setDirection(const glm::vec3& direction) {
    Coordination::setDirection(direction);
    updateViewMatrix();
}