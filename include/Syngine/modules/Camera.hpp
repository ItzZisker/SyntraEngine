#pragma once

#include <Syngine/Syngine.hpp>
#include <Syngine/world/World.hpp>
#include <Syngine/world/WorldObject.hpp>
#include <Syngine/modules/Shader.hpp>

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

    void setUp(const glm::vec3& up) override;

    glm::mat4 getViewMatrix();

    void updateViewMatrix();
private:
    glm::mat4 viewMatrix;
};