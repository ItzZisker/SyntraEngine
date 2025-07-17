#pragma once

#include "world/World.hpp"
#include "world/entity/BT_Entity.hpp"
#include <engine/RenderTable.hpp>
#include <modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

namespace syng
{
class BT_EntityTriangleMesh : public syng::BT_Entity
{
private:
    Mesh* mesh;
    btTriangleMesh* triangleMesh;
    btBvhTriangleMeshShape* shape;
public:
    Coordination coords;
    float mass;

    BT_EntityTriangleMesh(BT_World* world, float mass, Mesh* mesh);

    ~BT_EntityTriangleMesh();

    const glm::mat4 onMotionState() override;

    void load(bool useQuantiziedAabbCompression = true);

    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    Mesh* getMesh() {
        return this->mesh;
    }
};
}