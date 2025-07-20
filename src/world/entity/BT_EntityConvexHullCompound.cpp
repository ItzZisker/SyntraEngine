#include <LinearMath/btVector3.h>

#include <modules/Mesh.hpp>
#include <world/entity/BT_EntityConvexHullCompound.hpp>
#include "utils/GameUtils.hpp"

#include <iostream>
#include <string>
#include <unordered_map>

using namespace syng;

BT_EntityConvexHullCompound::BT_EntityConvexHullCompound(BT_World* world, float mass, Model* model) 
    : BT_Entity(world), mass(mass), meshes(model->meshes) {}

BT_EntityConvexHullCompound::BT_EntityConvexHullCompound(BT_World* world, float mass, std::unordered_map<std::string, Mesh*> meshes) 
    : BT_Entity(world), mass(mass), meshes(meshes) {}

BT_EntityConvexHullCompound::~BT_EntityConvexHullCompound() {
    worldAsBT()->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

const glm::mat4 BT_EntityConvexHullCompound::onMotionState() {
    btTransform bulletTransform;
    body->getMotionState()->getWorldTransform(bulletTransform);
    return GameUtils::fromBulletTransform(bulletTransform); // worldTransform
}

void BT_EntityConvexHullCompound::load(bool enablePolyhedral) {
    for (const auto& it : meshes) {
        if (!it.second->loaded) {
            std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
            return;
        }
    }

    shape = new btCompoundShape();
    int totalPoints = 0;

    for (const auto& it : meshes) {
        Mesh* mesh = it.second;

        std::vector<btVector3> points;
        for (const Vertex& vertex : mesh->vertices) {
            glm::vec3 transformed = glm::vec3(mesh->getParentToNodeTransform() * glm::vec4(vertex.position, 1.0f));
            points.emplace_back(transformed.x, transformed.y, transformed.z);
        }

        btConvexHullShape* subShape = new btConvexHullShape();
        for (const btVector3& p : points) {
            subShape->addPoint(p, false);
        }
        subShape->recalcLocalAabb();

        btTransform meshTransform;
        
        meshTransform.setIdentity();
        meshTransform.setFromOpenGLMatrix(glm::value_ptr(mesh->getParentToNodeTransform()));

        shape->addChildShape(meshTransform, subShape);
    }
    shape->recalculateLocalAabb();

    btTransform startTransform;
    startTransform.setFromOpenGLMatrix(glm::value_ptr(coords.getTransform()));

    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
}