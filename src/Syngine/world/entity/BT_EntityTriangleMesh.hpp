#pragma once
#ifdef USE_BULLET

#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/world/World.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/world/entity/BT_Entity.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Model.hpp"

#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
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

#endif