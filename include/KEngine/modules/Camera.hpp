#pragma once

#include <KEngine/KEngine.hpp>
#include <KEngine/world/World.hpp>
#include <KEngine/world/WorldObject.hpp>
#include <KEngine/modules/Shader.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public WorldObject
{
public:
    Camera(World* world, glm::vec3 position, glm::vec3 target, glm::vec3 up);

    Camera(World* world, glm::vec3 position, float yaw, float pitch, glm::vec3 up);

    Camera(World* world, glm::vec3 position, glm::vec3 target);

    Camera(World* world, glm::vec3 position, float yaw, float pitch);

    void render(GameWindow *window) override;

    void setDirection(glm::vec3 direction) override;

    void setPosition(glm::vec3 position) override;

    glm::mat4 getViewMatrix();

    glm::vec3 getUp();

    void updateViewMatrix();

    void setUp(glm::vec3 up);
private:
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 viewMatrix;
};