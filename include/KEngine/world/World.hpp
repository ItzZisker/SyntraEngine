#pragma once

#include <KEngine/KEngine.hpp>
#include <KEngine/engine/RenderTable.hpp>

#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>

#include <glm/glm.hpp>
#include <string>

class World : public Renderable {
private:
    btDynamicsWorld* dynamicsWorld;
public:
    const unsigned int id;
    const std::string name;

    bool paused = true;

    World(unsigned int id, std::string name, glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f)) : id(id), name(name) {
        btBroadphaseInterface* broadphase = new btDbvtBroadphase();

        btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

        dynamicsWorld = new btDiscreteDynamicsWorld(
            dispatcher,
            broadphase,
            solver,
            collisionConfiguration
        );
        dynamicsWorld->setGravity(btVector3(gravity[0], gravity[1], gravity[2]));
    }

    void render(GameWindow* window) override {
        if (!paused)
            dynamicsWorld->stepSimulation(window->getLastFrameTime());
    }

    btDynamicsWorld* getDynamics() {
        return this->dynamicsWorld;
    }
};