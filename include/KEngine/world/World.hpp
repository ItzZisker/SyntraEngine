#pragma once

#include <KEngine/KEngine.hpp>
#include <KEngine/engine/RenderTable.hpp>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <glm/glm.hpp>
#include <string>

class World : public WindowRenderable {
private:
    btDynamicsWorld* dynamicsWorld;
public:
    const unsigned int id;
    const std::string name;

    bool paused = true;

    World(unsigned int id, std::string name, glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f));

    void render(GameWindow* window, int parentFBO) override;

    btDynamicsWorld* getDynamics() {
        return this->dynamicsWorld;
    }
};