#include "KEngine/world/WorldObject.hpp"
#include <KEngine/modules/Mesh.hpp>
#include <LinearMath/btVector3.h>
#include <KEngine/world/entity/EntityConvexHull.hpp>
#include <iostream>

EntityConvexHull::EntityConvexHull(World* world, float mass, Model* model) : WorldObject(world), mass(mass), model(model) {}

EntityConvexHull::~EntityConvexHull() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

void EntityConvexHull::load(bool enablePolyhedral) {
    if (!model->loaded) {
        std::cerr << "ERROR::Entity::<UNLOADED_MODEL>" << std::endl;
        return;
    }

    int numPoints = 0;
    for (const Mesh& mesh : model->meshes) {
        numPoints += mesh.vertices.size();
    }

    float* points = new float[3 * numPoints];

    long captured = GameUtils::currentTime();
    int i = 0;
    for (const Mesh& mesh : model->meshes) {
        for (const Vertex& vertex : mesh.vertices) {
            glm::vec3 vec = vertex.position;
            points[i++] = vec[0];
            points[i++] = vec[1];
            points[i++] = vec[2];
        }
    }
    shape = new btConvexHullShape(points, numPoints, 3 * sizeof(float));   

    if (enablePolyhedral){
        shape->initializePolyhedralFeatures();
    }

    glm::vec3 modelPos = model->getPosition();
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(
            GameUtils::getBulletRotationFromTransform(model->getTransform()),
            btVector3(modelPos.x, modelPos.y, modelPos.z)
        )
    );

    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
    if (hasRollingFriction) body->setRollingFriction(rollingFriction);
    body->setFriction(1.0f);
    body->setDamping(0.8f, 0.2f);

    world->getDynamics()->addRigidBody(body);
}

void EntityConvexHull::render(GameWindow* window) {
    model->render(window);

    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    model->setTransform(GameUtils::fromBulletTransform(transform));
}