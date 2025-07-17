#pragma once

#include <world/entity/BT_Entity.hpp>
#include <world/WorldObject.hpp>
#include <engine/RenderTable.hpp>
#include <modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

namespace syng
{

class BT_EntityConvexHull : public syng::BT_Entity
{
private:
    Mesh* mesh;
    btConvexHullShape* shape;
public:
    Coordination coords;
    bool hasRollingFriction = true;
    float mass;
    float friction = 1.0f, rollingFriction = 0.3f, linearDamping = 0.8f, angularDamping = 0.2f;

    BT_EntityConvexHull(syng::BT_World* world, float mass, Mesh* mesh);

    ~BT_EntityConvexHull();

    const glm::mat4 onMotionState() override;

    void load(bool enablePolyhedral = true);

    btConvexHullShape* getShape() {
        return this->shape;
    }

    Mesh* getMesh() {
        return this->mesh;
    }
};

}