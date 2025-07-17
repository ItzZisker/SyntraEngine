#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include <world/World.hpp>
#include <utils/GameUtils.hpp>

#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

void syng::BT_World::create(unsigned int id, std::string name, glm::vec3 gravity) {
    if (this->dynamicsWorld) return;

    this->id = id;
    this->name = name;
    this->gravity = gravity;

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
    dynamicsWorld->setGravity(syng::GameUtils::toBulletVector(gravity));
}

syng::BT_World::BT_World(unsigned int id, std::string name, glm::vec3 gravity) {
    create(id, name, gravity);
}

void syng::BT_World::render(GameWindow* window) {
    if (!paused) {
        dynamicsWorld->stepSimulation(window->getLastFrameTime());
    }
}

btDynamicsWorld* syng::BT_World::getDynamics() {
    return this->dynamicsWorld;
}