#include <unordered_map>
#include <world/entity/BT_EntityConvexHull.hpp>
#include "modules/MeshInstance.hpp"
#include "modules/ModelInstance.hpp"
#include "world/World.hpp"
#include "world/WorldObject.hpp"
#include <LinearMath/btVector3.h>
#include <modules/Mesh.hpp>
#include "utils/GameUtils.hpp"
#include <iostream>

using namespace syng;

BT_EntityConvexHull::BT_EntityConvexHull(BT_World* world, float mass, MeshInstance* mesh) 
    : BT_Entity(world), mass(mass), meshes({{"", mesh}}) {}

BT_EntityConvexHull::BT_EntityConvexHull(BT_World* world, float mass, ModelInstance* model) 
    : BT_Entity(world), mass(mass), meshes(model->getMeshInstances()->asMap()) {}

BT_EntityConvexHull::BT_EntityConvexHull(BT_World* world, float mass, std::unordered_map<std::string, MeshInstance*> meshes) 
    : BT_Entity(world), mass(mass), meshes(meshes) {}

BT_EntityConvexHull::~BT_EntityConvexHull() {
    worldAsBT()->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

const glm::mat4 BT_EntityConvexHull::onMotionState() {
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    return GameUtils::fromBulletTransform(transform);
}

void BT_EntityConvexHull::load(bool enablePolyhedral) {
    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        if (!instance->getMesh()->loaded) {
            std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
            return;
        }
    }

    int numPoints = 0;
    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        numPoints += instance->getMesh()->vertices.size();
    }

    float* points = new float[3 * numPoints];

    int i = 0;
    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        Mesh *mesh = instance->getMesh();

        for (const Vertex& vertex : mesh->vertices) {
            glm::vec4 vec = glm::vec4(vertex.position, 1.0f) * mesh->getParentToNodeTransform() * instance->getTransform();
            points[i++] = vec[0];
            points[i++] = vec[1];
            points[i++] = vec[2];
        }
    }
    shape = new btConvexHullShape(points, numPoints, 3 * sizeof(float));   
    shape->setMargin(0.05f);

    if (enablePolyhedral){
        shape->initializePolyhedralFeatures();
    }

    btDefaultMotionState* motionState = new btDefaultMotionState( btTransform(
        GameUtils::getBulletRotationFromTransform(coords.getTransform()),
        GameUtils::toBulletVector(coords.getPosition())
    ));

    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);

    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
    if (hasRollingFriction) body->setRollingFriction(rollingFriction);
    body->setFriction(1.0f);
    body->setDamping(0.8f, 0.2f);

    worldAsBT()->getDynamics()->addRigidBody(body);
}