#include <LinearMath/btVector3.h>

#include <Syngine/modules/Mesh.hpp>
#include <Syngine/world/entity/EntityConvexHullCompound.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>

EntityConvexHullCompound::EntityConvexHullCompound(World* world, float mass, Model* model) 
    : Entity(world), mass(mass), meshes(model->meshes) {}

EntityConvexHullCompound::EntityConvexHullCompound(World* world, float mass, std::unordered_map<std::string, Mesh*> meshes) 
    : Entity(world), mass(mass), meshes(meshes) {}

EntityConvexHullCompound::~EntityConvexHullCompound() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
}

const glm::mat4 EntityConvexHullCompound::onMotionState() {
    btTransform bulletTransform;
    body->getMotionState()->getWorldTransform(bulletTransform);
    return GameUtils::fromBulletTransform(bulletTransform); // worldTransform
}

void EntityConvexHullCompound::load(bool enablePolyhedral) {
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
            glm::vec3 transformed = glm::vec3(mesh->getTransform() * glm::vec4(vertex.position, 1.0f));
            points.emplace_back(transformed.x, transformed.y, transformed.z);
        }

        btConvexHullShape* subShape = new btConvexHullShape();
        for (const btVector3& p : points) {
            subShape->addPoint(p, false);
        }
        subShape->recalcLocalAabb();

        btTransform meshTransform;
        
        meshTransform.setIdentity();
        meshTransform.setFromOpenGLMatrix(glm::value_ptr(mesh->getTransform()));

        shape->addChildShape(meshTransform, subShape);
    }
    shape->recalculateLocalAabb();

    btTransform startTransform;
    startTransform.setFromOpenGLMatrix(glm::value_ptr(coords.getTransform()));

    btVector3 inertia(0,0,0);
    shape->calculateLocalInertia(mass, inertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));

    std::unordered_map<std::string, Mesh*> meshesCopy = meshes;
    addMotionState("DEFAULT", [meshesCopy](const glm::mat4& transform) {        
        for (const auto& it : meshesCopy) {
            Mesh* mesh = it.second;
            mesh->setTransform(transform * mesh->getTransform());
        }
    });
}