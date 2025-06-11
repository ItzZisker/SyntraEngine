#pragma once

#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/world/entity/Entity.hpp"
#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <string>
#include <unordered_map>

class EntityTriangleMeshCompound : public Entity
{
private:
    std::unordered_map<std::string, Mesh*> meshes;
    btTriangleMesh* triangleMesh;
    btTriangleInfoMap* triangleInfoMap;
    btBvhTriangleMeshShape* shape;
public:
    CoordinatedObject coords;

    EntityTriangleMeshCompound(World* world, Model* model);

    EntityTriangleMeshCompound(World* world, std::unordered_map<std::string, Mesh*> meshes);

    ~EntityTriangleMeshCompound();

    const glm::mat4 onMotionState() override;

    void load(bool useQuantiziedAabbCompression = true);
    
    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    const std::unordered_map<std::string, Mesh*>& getMeshes() {
        return this->meshes;
    }
};