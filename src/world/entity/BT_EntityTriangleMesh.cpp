#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "world/World.hpp"
#include "world/WorldObject.hpp"
#include <modules/Mesh.hpp>
#include <LinearMath/btVector3.h>
#include <world/entity/BT_EntityTriangleMesh.hpp>
#include "utils/GameUtils.hpp"

#include <iostream>

using namespace syng;

BT_EntityTriangleMesh::BT_EntityTriangleMesh(BT_World* world, float mass, Mesh* mesh)
    : BT_Entity(world), mass(mass), mesh(mesh) {}

BT_EntityTriangleMesh::~BT_EntityTriangleMesh() {
    worldAsBT()->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
    delete triangleMesh;
}

const glm::mat4 BT_EntityTriangleMesh::onMotionState() {
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    return GameUtils::fromBulletTransform(transform);
}

void BT_EntityTriangleMesh::load(bool useQuantizedAabbCompression) {
    if (!mesh->loaded) {
        std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
        return;
    }

    triangleMesh = new btTriangleMesh();

    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        Vertex v0 = mesh->vertices[mesh->indices[i]];
        Vertex v1 = mesh->vertices[mesh->indices[i + 1]];
        Vertex v2 = mesh->vertices[mesh->indices[i + 2]];

        triangleMesh->addTriangle(
            GameUtils::toBulletVector(v0.position),
            GameUtils::toBulletVector(v1.position),
            GameUtils::toBulletVector(v2.position)
        );
    }

    shape = new btBvhTriangleMeshShape(triangleMesh, useQuantizedAabbCompression);

    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(
        GameUtils::getBulletRotationFromTransform(coords.getTransform()),
        GameUtils::toBulletVector(coords.getPosition())
    ));

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, btVector3(0, 0, 0)));
    worldAsBT()->getDynamics()->addRigidBody(body);
}