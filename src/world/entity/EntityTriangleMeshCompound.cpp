#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include <KEngine/modules/Mesh.hpp>
#include <LinearMath/btVector3.h>
#include <KEngine/world/entity/EntityTriangleMeshCompound.hpp>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

#include <iostream>

EntityTriangleMeshCompound::EntityTriangleMeshCompound(World* world, Model* model) 
    : Entity(world), meshes(model->meshes) {}

EntityTriangleMeshCompound::EntityTriangleMeshCompound(World* world, std::unordered_map<std::string, Mesh*> meshes) 
    : Entity(world), meshes(meshes) {}

EntityTriangleMeshCompound::~EntityTriangleMeshCompound() {
    world->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
    delete triangleInfoMap;
    delete triangleMesh;
}

const glm::mat4 EntityTriangleMeshCompound::onMotionState() {
    return coords.getTransform();
}

void EntityTriangleMeshCompound::load(bool useQuantizedAabbCompression) {
    for (const auto& it : meshes) {
        if (!it.second->loaded) {
            std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
            return;
        }
    }
    triangleMesh = new btTriangleMesh();
    triangleInfoMap = new btTriangleInfoMap();

    for (const auto& it : meshes) {
        Mesh* mesh = it.second;
        glm::mat4 transform = mesh->getTransform();

        const auto& vertices = mesh->vertices;
        const auto& indices = mesh->indices;

        for (size_t i = 0; i < indices.size(); i += 3) {
            glm::vec3 v0 = glm::vec3(transform * glm::vec4(vertices[indices[i]].position, 1.0f));
            glm::vec3 v1 = glm::vec3(transform * glm::vec4(vertices[indices[i + 1]].position, 1.0f));
            glm::vec3 v2 = glm::vec3(transform * glm::vec4(vertices[indices[i + 2]].position, 1.0f));

            triangleMesh->addTriangle(
                btVector3(v0.x, v0.y, v0.z),
                btVector3(v1.x, v1.y, v1.z),
                btVector3(v2.x, v2.y, v2.z),
                true
            );
        }
    }

    shape = new btBvhTriangleMeshShape(triangleMesh, useQuantizedAabbCompression);
    shape->setMargin(0.05f);
    btGenerateInternalEdgeInfo(static_cast<btBvhTriangleMeshShape*>(shape), triangleInfoMap);

    btTransform startTransform;
    startTransform.setFromOpenGLMatrix(glm::value_ptr(coords.getTransform()));

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, motionState, shape, btVector3(0, 0, 0));

    body = new btRigidBody(rigidBodyCI);
    body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

    world->getDynamics()->addRigidBody(body);

    std::unordered_map<std::string, Mesh*> meshesCopy = meshes;
    addMotionState("DEFAULT", [meshesCopy](const glm::mat4& transform) {        
        for (const auto& it : meshesCopy) {
            Mesh* mesh = it.second;
            mesh->setTransform(transform * mesh->getTransform());
        }
    });
}