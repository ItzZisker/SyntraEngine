#include "glm/fwd.hpp"
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
                 sin(glm::radians(yaw)) * cos(glm::radians(pitch))),
             up) {}

Camera::Camera(World* world, glm::vec3 position, glm::vec3 target)
    : Camera(world, position, target, glm::vec3(0, 1, 0)) {}

Camera::Camera(World* world, glm::vec3 position, float yaw, float pitch)
    : Camera(world, position, yaw, pitch, glm::vec3(0, 1, 0)) {}

void Camera::updateViewMatrix() {
    glm::vec3 pos = getPosition();
    glm::vec3 dir = getDirection();
    viewMatrix = glm::lookAt(pos, pos + dir, up);
}

glm::mat4 Camera::getViewMatrix() {
    return viewMatrix;
}

glm::vec3 Camera::getUp() {
    return glm::normalize(glm::vec3(transform[1]));
}

void Camera::setUp(glm::vec3 newUp) {
    glm::vec3 forward = getDirection();
    glm::vec3 right = glm::normalize(glm::cross(newUp, forward));
    glm::vec3 up = glm::normalize(glm::cross(forward, right));

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(-forward, 0.0f);
    rotation[3] = transform[3];

    transform = rotation;
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