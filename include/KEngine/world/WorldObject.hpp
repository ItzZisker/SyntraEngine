#pragma once

#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/world/World.hpp>
#include <KEngine/utils/GameUtils.hpp>

#include <glm/glm.hpp>

class CoordinatedObject : public Renderable {
protected:
    glm::vec3 position, direction;
    float roll;
public:
    virtual float getYaw() {
        return direction.x == 0 ? (direction.z >= 0 ? 90.0f : 270.0f) : glm::atan(direction.z / direction.x);
    }

    virtual float getPitch() { 
        return glm::asin(direction.y);
    }

    virtual float getRoll() {
        return roll;
    }

    virtual glm::vec3 getPosition() { 
        return glm::vec3(position);
    }

    virtual glm::vec3 getDirection() {
        return glm::vec3(direction);
    }

    virtual void setYaw(float yaw) {
        setDirection(GameUtils::directionOf(yaw, getPitch()));
    }

    virtual void setPitch(float pitch) {
        setDirection(GameUtils::directionOf(getYaw(), pitch));
    }

    virtual void setRoll(float roll) {
        this->roll = roll;
    }

    virtual void setEuler(float yaw, float pitch, float roll) {
        setDirection(GameUtils::directionOf(yaw, pitch));
        this->roll = roll;
    }

    virtual void setDirection(glm::vec3 direction) {
        this->direction = direction;
    }

    virtual void setPosition(glm::vec3 position) {
        this->position = position;
    }
};

class WorldObject : public CoordinatedObject {
protected:
    World *world;
public:
    WorldObject(World *initialWorld) : world(initialWorld) {}

    virtual void render(GameWindow *window) = 0;

    void setWorld(World *world) {
        this->world = world; 
    }

    World *getWorld() const { 
        return world;
    }
};