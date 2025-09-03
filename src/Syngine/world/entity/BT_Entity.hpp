#pragma once

#ifdef USE_BULLET
#include "BulletDynamics/Dynamics/btRigidBody.h"
#endif

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glm/fwd.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace syng
{

#ifdef USE_BULLET
class BT_Entity : public Coordination, public WindowRenderable {
protected:
    btRigidBody* body;
    std::unordered_map<std::string, std::function<void(const glm::mat4&)>> motionStateFunctions;
public:
    btRigidBody* getBody() {
        return this->body;
    }

    void bind(std::string motionKey, MeshInstance* meshInstance) {
        if (hasMotionState(motionKey)) {
            std::cerr << "Already has MotionState: " << motionKey << std::endl;
        } else {
            addMotionState(motionKey, [meshInstance](const glm::mat4& m) {
                glm::vec3 scale = meshInstance->getScale();
                meshInstance->setTransform(m);
                meshInstance->setScale(scale);
            });
        }
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

    void render(GameWindow* window) override {
        const glm::mat4& transform = onMotionState();
        for (const auto& func : motionStateFunctions) {
            func.second(transform);
        }
    }
};
#endif

}