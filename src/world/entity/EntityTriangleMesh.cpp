#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "Syngine/world/WorldObject.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <LinearMath/btVector3.h>
#include <Syngine/world/entity/EntityTriangleMesh.hpp>

#include <iostream>

EntityTriangleMesh::EntityTriangleMesh(World* world, float mass, Mesh* mesh)
    : Entity(world), mass(mass), mesh(mesh) {}

EntityTriangleMesh::~EntityTriangleMesh() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
    delete triangleMesh;
}

const glm::mat4 EntityTriangleMesh::onMotionState() {
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    return GameUtils::fromBulletTransform(transform);
}

void EntityTriangleMesh::load(bool useQuantizedAabbCompression) {
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
        GameUtils::getBulletRotationFromTransform(mesh->getTransform()),
        GameUtils::toBulletVector(mesh->getPosition())
    ));

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, btVector3(0, 0, 0)));
    world->getDynamics()->addRigidBody(body);

    Mesh* meshCopy = mesh;
    addMotionState("DEFAULT", [meshCopy](const glm::mat4& transform) {
        meshCopy->setTransform(transform);
    });
}