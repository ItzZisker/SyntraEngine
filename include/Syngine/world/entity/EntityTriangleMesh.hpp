#pragma once

#include "Syngine/world/entity/Entity.hpp"
#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

class EntityTriangleMesh : public Entity
{
private:
    Mesh* mesh;
    btTriangleMesh* triangleMesh;
    btBvhTriangleMeshShape* shape;
public:
    Coordination coords;
    float mass;

    EntityTriangleMesh(World* world, float mass, Mesh* mesh);

    ~EntityTriangleMesh();

    const glm::mat4 onMotionState() override;

    void load(bool useQuantiziedAabbCompression = true);

    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    Mesh* getMesh() {
        return this->mesh;
    }
};