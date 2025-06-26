#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <Syngine/modules/Camera.hpp>

Camera::Camera(World* world, glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : WorldObject(world) {
    glm::vec3 dir = glm::normalize(target - position);
    setDirection(dir);
    setUp(up);
    setPosition(position);
    updateViewMatrix();
}

Camera::Camera(World* world, glm::vec3 position, float yaw, float pitch, glm::vec3 up)
    : Camera(world, position,
             glm::vec3(
                 cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                 sin(glm::radians(pitch)),
                 sin(glm::radians(yaw)) * cos(glm::radians(pitch)))) {
    setUp(up);
}

Camera::Camera(World* world, glm::vec3 position, glm::vec3 target)
    : Camera(world, position, target, glm::vec3(0, 1, 0)) {}

Camera::Camera(World* world, glm::vec3 position, float yaw, float pitch)
    : Camera(world, position, yaw, pitch, glm::vec3(0, 1, 0)) {}

void Camera::updateViewMatrix() {
    glm::vec3 pos = getPosition();
    viewMatrix = glm::lookAt(pos, pos + getDirection(), getUp());
}

glm::mat4 Camera::getViewMatrix() {
    return viewMatrix;
}

void Camera::setUp(const glm::vec3& newUp) {
    WorldObject::setUp(newUp);
    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3& position) {
    WorldObject::setPosition(position);
    updateViewMatrix();
}

void Camera::setDirection(const glm::vec3& direction) {
    WorldObject::setDirection(direction);
    updateViewMatrix();
}