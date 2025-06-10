#pragma once

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "KEngine/engine/RenderTable.hpp"
#include "KEngine/modules/Shader.hpp"
#include "KEngine/world/WorldObject.hpp"
#include "glm/fwd.hpp"

#include <functional>
#include <string>
#include <unordered_map>

class Entity : public WorldObject, public WindowRenderable {
protected:
    btRigidBody* body;
    std::unordered_map<std::string, std::function<void(const glm::mat4&)>> motionStateFunctions;
public:
    Entity(World* world) : WorldObject(world) {}

    btRigidBody* getBody() {
        return this->body;
    }

    void addMotionState(const std::string& key, std::function<void(const glm::mat4&)> func) {
        motionStateFunctions.insert({key, func});
    }

    void removeMotionState(const std::string& key) {
        motionStateFunctions.erase(key);
    }

    bool hasMotionState(const std::string& key) const {
        return motionStateFunctions.find(key) != motionStateFunctions.end();
    }

    virtual const glm::mat4 onMotionState() = 0;

    void render(GameWindow* window, int parentFBO) override {
        const glm::mat4& transform = onMotionState();
        for (const auto& func : motionStateFunctions) {
            func.second(transform);
        }
    }
};