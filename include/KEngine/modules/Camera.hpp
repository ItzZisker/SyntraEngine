#pragma once

#include <KEngine/KEngine.hpp>
#include <KEngine/world/World.hpp>
#include <KEngine/world/WorldObject.hpp>
#include <KEngine/modules/Shader.hpp>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public WorldObject {
public:
    Camera(World* world, glm::vec3 position, glm::vec3 target, glm::vec3 up);
    
    Camera(World* world, glm::vec3 position, float yaw, float pitch, glm::vec3 up);
    
    Camera(World* world, glm::vec3 position, glm::vec3 target);

    Camera(World* world, glm::vec3 position, float yaw, float pitch);

    void setDirection(const glm::vec3& direction) override;
    
    void setPosition(const glm::vec3& position) override;

    glm::mat4 getViewMatrix();

    glm::vec3 getUp();

    void setUp(glm::vec3 up);

    void updateViewMatrix();
private:
    glm::mat4 viewMatrix;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
};