#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include <KEngine/modules/Mesh.hpp>
#include <LinearMath/btVector3.h>
#include <KEngine/world/entity/EntityTriangleMesh.hpp>
#include <iostream>

EntityTriangleMesh::EntityTriangleMesh(World* world, float mass, Model* model) : WorldObject(world), mass(mass), model(model) {}

EntityTriangleMesh::~EntityTriangleMesh() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

void EntityTriangleMesh::load(bool useQuantizedAabbCompression) {
    if (!model->loaded) {
        std::cerr << "ERROR::Entity::<UNLOADED_MODEL>" << std::endl;
        return;
    }

    btTriangleMesh* triangleMesh = new btTriangleMesh();

    for (const Mesh* mesh : model->meshes) {
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
    }

    shape = new btBvhTriangleMeshShape(triangleMesh, useQuantizedAabbCompression);

    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(
        GameUtils::getBulletRotationFromTransform(model->getTransform()),
        GameUtils::toBulletVector(model->getPosition())
    ));

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, btVector3(0, 0, 0)));
    world->getDynamics()->addRigidBody(body);
}

void EntityTriangleMesh::render(GameWindow* window) {
    model->render(window);

    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    model->setTransform(GameUtils::fromBulletTransform(transform));
}