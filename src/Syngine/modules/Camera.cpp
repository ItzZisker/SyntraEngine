#include "Camera.hpp"

#include "Syngine/world/Coordination.hpp"

#include "glm/fwd.hpp"

#include <glm/ext/matrix_transform.hpp>

using namespace syng;

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    setPosition(position);
    setDirection(glm::normalize(target - position), up);
    updateViewMatrix();
}

Camera::Camera(glm::vec3 position, float yaw, float pitch, glm::vec3 up)
    : Camera(position, glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
)) {}

Camera::Camera(glm::vec3 position, glm::vec3 target)
    : Camera(position, target, glm::vec3(0, 1, 0)) {}

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : Camera(position, yaw, pitch, glm::vec3(0, 1, 0)) {}

void Camera::updateViewMatrix() {
    viewMatrix = glm::mat4_cast(glm::conjugate(coords.getRotation()));
    viewMatrix = glm::translate(viewMatrix, -coords.getPosition());
}

void Camera::setPosition(const glm::vec3& position) {
    coords.setPosition(position);
    updateViewMatrix();
}

void Camera::setDirection(const glm::vec3& direction, const glm::vec3 upHint) {
    coords.setRotationTowardsDirection(direction);
    updateViewMatrix();
}

glm::vec3 Camera::getPosition() {
    return coords.getPosition();
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