#pragma once

#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include "world/World.hpp"
#include "world/WorldObject.hpp"
#include "world/entity/BT_Entity.hpp"
#include <engine/RenderTable.hpp>
#include <modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <string>
#include <unordered_map>

namespace syng
{
class BT_EntityTriangleMeshCompound : public BT_Entity
{
private:
    std::unordered_map<std::string, Mesh*> meshes;
    btTriangleMesh* triangleMesh;
    btTriangleInfoMap* triangleInfoMap;
    btBvhTriangleMeshShape* shape;
public:
    Coordination coords;

    BT_EntityTriangleMeshCompound(BT_World* world, Model* model);

    BT_EntityTriangleMeshCompound(BT_World* world, std::unordered_map<std::string, Mesh*> meshes);

    ~BT_EntityTriangleMeshCompound();

    const glm::mat4 onMotionState() override;

    void load(bool useQuantiziedAabbCompression = true);
    
    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, Mesh*>& getMeshes() {
        return this->meshes;
    }
};
}