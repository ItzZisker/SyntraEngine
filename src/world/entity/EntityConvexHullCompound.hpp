#pragma once

#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include <world/entity/Entity.hpp>
#include <world/WorldObject.hpp>
#include <engine/RenderTable.hpp>
#include <modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <unordered_map>

class EntityConvexHullCompound : public Entity
{
private:
    std::unordered_map<std::string, Mesh*> meshes;
    btCompoundShape* shape;
public:
    Coordination coords;
    bool hasRollingFriction = true;
    float mass;
    float friction = 1.0f, rollingFriction = 0.3f, linearDamping = 0.8f, angularDamping = 0.2f;

    EntityConvexHullCompound(World* world, float mass, Model* model);

    EntityConvexHullCompound(World* world, float mass, std::unordered_map<std::string, Mesh*> meshes);

    ~EntityConvexHullCompound();

    const glm::mat4 onMotionState() override;

    void load(bool enablePolyhedral = true);

    btCompoundShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, Mesh*>& getMesh() {
        return this->meshes;
    }
};