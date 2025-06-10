#include "KEngine/world/WorldObject.hpp"

#include <LinearMath/btVector3.h>

#include <KEngine/modules/Mesh.hpp>
#include <KEngine/world/entity/EntityConvexHull.hpp>

#include <cmath>
#include <iostream>

EntityConvexHull::EntityConvexHull(World* world, float mass, Mesh* mesh) 
    : Entity(world), mass(mass), mesh(mesh) {}

EntityConvexHull::~EntityConvexHull() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

const glm::mat4 EntityConvexHull::onMotionState() {
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    return GameUtils::fromBulletTransform(transform);
}

void EntityConvexHull::load(bool enablePolyhedral) {
    if (!mesh->loaded) {
        std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
        return;
    }

    int numPoints = mesh->vertices.size();
    float* points = new float[3 * numPoints];

    int i = 0;
    for (const Vertex& vertex : mesh->vertices) {
        glm::vec3 vec = vertex.position;
        points[i++] = vec[0];
        points[i++] = vec[1];
        points[i++] = vec[2];
    }
    shape = new btConvexHullShape(points, numPoints, 3 * sizeof(float));   
    shape->setMargin(0.05f);

    if (enablePolyhedral){
        shape->initializePolyhedralFeatures();
    }

    btDefaultMotionState* motionState = new btDefaultMotionState( btTransform(
        GameUtils::getBulletRotationFromTransform(mesh->getTransform()),
        GameUtils::toBulletVector(mesh->getPosition())
    ));

    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
    if (hasRollingFriction) body->setRollingFriction(rollingFriction);
    body->setFriction(1.0f);
    body->setDamping(0.8f, 0.2f);

    world->getDynamics()->addRigidBody(body);

    Mesh* meshCopy = mesh;
    addMotionState("DEFAULT", [meshCopy](const glm::mat4& transform) {
        meshCopy->setTransform(transform);
    });
}