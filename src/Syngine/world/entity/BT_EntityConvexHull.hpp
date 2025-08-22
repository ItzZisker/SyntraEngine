#pragma once
#ifdef USE_BULLET

#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/World.hpp"
#include "Syngine/world/entity/BT_Entity.hpp"
#include "Syngine/world/WorldObject.hpp"

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

#include <unordered_map>
#include <string>

namespace syng
{
class BT_EntityConvexHull : public syng::BT_Entity
{
private:
    std::unordered_map<std::string, MeshInstance*> meshes;
    btConvexHullShape* shape;
public:
    Coordination coords;
    bool hasRollingFriction = true;
    float mass;
    float friction = 1.0f, rollingFriction = 0.3f, linearDamping = 0.8f, angularDamping = 0.2f;

    BT_EntityConvexHull(BT_World* world, float mass, ModelInstance* model);
    BT_EntityConvexHull(BT_World* world, float mass, MeshInstance* mesh);
    ~BT_EntityConvexHull();

    const glm::mat4 onMotionState() override;

    void load(bool enablePolyhedral = true);

    btConvexHullShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, MeshInstance*>& getMeshes() {
        return this->meshes;
    }
};
}

#endif