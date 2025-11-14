#include "Camera.hpp"
#include "Syngine/world/Coordination.hpp"
#include "glm/fwd.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <iostream>

using namespace syng;

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    glm::vec3 dir = glm::normalize(target - position);
    setYaw(std::atan2(dir.x, dir.z) * 180.0f / M_PI);
    setPitch(std::atan2(-dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z) * 180.0f / M_PI));
    setUp(up);
    setPosition(position);
    updateViewMatrix();
}

Camera::Camera(glm::vec3 position, float yaw, float pitch, glm::vec3 up)
    : Camera(position, glm::vec3(
                 cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                 sin(glm::radians(pitch)),
                 sin(glm::radians(yaw)) * cos(glm::radians(pitch)))
            ) {
    setUp(up);
}

Camera::Camera(glm::vec3 position, glm::vec3 target)
    : Camera(position, target, glm::vec3(0, 1, 0)) {}

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : Camera(position, yaw, pitch, glm::vec3(0, 1, 0)) {}

void Camera::updateViewMatrix() {
    glm::vec3 pos = coords.getPosition();
    viewMatrix = glm::lookAt(pos, pos + coords.getDirection(), coords.getUp());
}

void Camera::setYaw(float degrees) {
    coords.setYaw(degrees);
    updateViewMatrix();
}

void Camera::setPitch(float degrees) {
    coords.setPitch(degrees);
    updateViewMatrix();
}

void Camera::setRoll(float degrees) {
    coords.setRoll(degrees);
    updateViewMatrix();
}

void Camera::setUp(const glm::vec3& newUp) {
    coords.setUp(newUp);
    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3& position) {
    coords.setPosition(position);
    updateViewMatrix();
}

float Camera::getYaw() {
    return coords.getYaw();
}

float Camera::getPitch() {
    return coords.getPitch();
}

float Camera::getRoll() {
    return coords.getRoll();
}

glm::vec3 Camera::getPosition() {
    return coords.getPosition();
}

glm::vec3 Camera::getDirection() {
    return coords.getDirection();
}

glm::vec3 Camera::getUp() {
    return coords.getUp();
}

glm::mat4 Camera::getTransform() {
    return coords.getTransform();
}

glm::mat4 Camera::getViewMatrix() {
    return viewMatrix;
}