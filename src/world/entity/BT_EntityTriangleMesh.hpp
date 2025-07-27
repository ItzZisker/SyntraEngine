#pragma once

#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include "modules/MeshInstance.hpp"
#include "modules/ModelInstance.hpp"
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
class BT_EntityTriangleMesh : public BT_Entity
{
private:
    std::unordered_map<std::string, MeshInstance*> meshes;
    btTriangleMesh* triangleMesh;
    btTriangleInfoMap* triangleInfoMap;
    btBvhTriangleMeshShape* shape;
public:
    Coordination coords;

    BT_EntityTriangleMesh(BT_World* world, ModelInstance* model);

    BT_EntityTriangleMesh(BT_World* world, std::unordered_map<std::string, MeshInstance*> meshes);

    BT_EntityTriangleMesh(BT_World* world, MeshInstance* mesh);

    ~BT_EntityTriangleMesh();

    const glm::mat4 onMotionState() override;

    void load(bool useQuantiziedAabbCompression = true);
    
    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, MeshInstance*>& getMeshes() {
        return this->meshes;
    }
};
}