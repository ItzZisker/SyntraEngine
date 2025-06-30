#include "BulletDynamics/Dynamics/btDynamicsWorld.h"
#include <Syngine/world/World.hpp>
#include <Syngine/utils/GameUtils.hpp>

#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

World::World(unsigned int id, std::string name, glm::vec3 gravity) : id(id), name(name) {
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
    dynamicsWorld->setGravity(GameUtils::toBulletVector(gravity));
}

void World::render(GameWindow* window) {
    if (!paused) {
        dynamicsWorld->stepSimulation(window->getLastFrameTime());
    }
}

btDynamicsWorld* World::getDynamics() {
    return this->dynamicsWorld;
}