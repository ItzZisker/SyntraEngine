#pragma once

#include <Syngine.hpp>
#include <engine/RenderTable.hpp>

#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <glm/glm.hpp>
#include <string>

namespace syng
{
class World : public WindowRenderable {
protected:
    unsigned int id;
    std::string name;
    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

    virtual void create(unsigned int id, std::string name, glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f)) = 0;
public:
    bool paused = true;

    unsigned int getID() const { return id; }
    
    std::string getName() const { return name; }

    glm::vec3 getGravity() const { return gravity; }
};

class BT_World : public World {
private:
    btDynamicsWorld* dynamicsWorld;
protected:
    void create(unsigned int id, std::string name, glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f)) override;
public:
    BT_World(unsigned int id, std::string name, glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f));

    void render(GameWindow* window) override;

    btDynamicsWorld* getDynamics();
};
// TODO: add PhysX world + Entities
}