#include <KEngine/modules/Camera.hpp>

#include <iostream>

Camera::Camera(kcomp::World* world, glm::vec3 position, glm::vec3 target, glm::vec3 up)
    : WorldObject(world), up(up)
{
    this->position = position;
    this->direction = glm::normalize(target - position);
    this->viewMatrix = glm::lookAt(position, target, up);
}

Camera::Camera(kcomp::World* world, glm::vec3 position, float yaw, float pitch, glm::vec3 up)
    : Camera(world, position,
             glm::vec3(
                 cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                 sin(glm::radians(pitch)),
                 sin(glm::radians(yaw)) * cos(glm::radians(pitch))),
             up) {}

Camera::Camera(kcomp::World* world, glm::vec3 position, glm::vec3 target)
    : Camera(world, position, target, glm::vec3(0.0f, 1.0f, 0.0f)) {}

Camera::Camera(kcomp::World* world, glm::vec3 position, float yaw, float pitch)
    : Camera(world, position, yaw, pitch, glm::vec3(0.0f, 1.0f, 0.0f)) {}

void Camera::render(kwindow::GameWindow *window)
{
    updateViewMatrix();

    Shader batchShader = window->getBatchShader();
    batchShader.use();
    batchShader.setMatrix4("view", viewMatrix, 1, GL_FALSE);
    batchShader.setVec3f("viewPos", position);
    batchShader.setVec3f("spotLight.position", position);
    batchShader.setVec3f("spotLight.direction", direction);
}

void Camera::updateViewMatrix()
{
    this->viewMatrix = glm::lookAt(position, position + direction, up);
}

glm::mat4 Camera::getViewMatrix()
{
    return this->viewMatrix;
}

glm::vec3 Camera::getUp()
{
    return this->up;
}

void Camera::setUp(glm::vec3 up)
{
    this->up = up;
    updateViewMatrix();
}

void Camera::setPosition(glm::vec3 position)
{
    kcomp::WorldObject::setPosition(position);
    updateViewMatrix();
}

void Camera::setDirection(glm::vec3 direction)
{
    kcomp::WorldObject::setDirection(direction);
    updateViewMatrix();
}