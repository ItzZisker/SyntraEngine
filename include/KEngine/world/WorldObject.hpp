#pragma once

#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/world/World.hpp>
#include <KEngine/utils/GameUtils.hpp>

#include <glm/glm.hpp>

class CoordinatedObject {
protected:
    glm::mat4 transform;
public:
    CoordinatedObject(glm::mat4 transform = glm::mat4(1.0f)) {
        this->transform = transform;
    }

    virtual const glm::mat4& getTransform() {
        return transform;
    }

    virtual glm::vec3 getPosition() const {
        return glm::vec3(transform[3]);
    }

    virtual glm::vec3 getDirection() const {
        return glm::normalize(glm::vec3(-transform[2]));
    }

    virtual float getYaw() const {
        glm::vec3 dir = getDirection();
        return glm::degrees(atan2(dir.x, -dir.z));
    }

    virtual float getPitch() const {
        return glm::degrees(asin(getDirection().y));
    }

    virtual void setTransform(const glm::mat4& transform) {
        this->transform = transform;
    }

    virtual void setPosition(const glm::vec3& pos) {
        transform[3] = glm::vec4(pos, 1.0f);
    }

    virtual void setDirection(const glm::vec3& dir) {
        glm::vec3 forward = glm::normalize(dir);
        glm::vec3 worldUp = glm::vec3(0, 1, 0);
        glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
        glm::vec3 up = glm::normalize(glm::cross(forward, right));

        glm::mat4 rotation(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);
        rotation[3] = transform[3];

        transform = rotation;
    }
};

class WorldObject : public CoordinatedObject {
protected:
    World *world;
public:
    WorldObject(World *initialWorld) : world(initialWorld) {}

    void setWorld(World *world) {
        this->world = world; 
    }

    World *getWorld() const { 
        return world;
    }
};