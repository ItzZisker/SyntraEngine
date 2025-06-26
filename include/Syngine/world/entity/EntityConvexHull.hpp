#pragma once

#include <Syngine/world/entity/Entity.hpp>
#include <Syngine/world/WorldObject.hpp>
#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

class EntityConvexHull : public Entity
{
private:
    Mesh* mesh;
    btConvexHullShape* shape;
public:
    Coordination coords;
    bool hasRollingFriction = true;
    float mass;
    float friction = 1.0f, rollingFriction = 0.3f, linearDamping = 0.8f, angularDamping = 0.2f;

    EntityConvexHull(World* world, float mass, Mesh* mesh);

    ~EntityConvexHull();

    const glm::mat4 onMotionState() override;

    void load(bool enablePolyhedral = true);

    btConvexHullShape* getShape() {
        return this->shape;
    }

    Mesh* getMesh() {
        return this->mesh;
    }
};