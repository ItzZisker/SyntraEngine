#include <modules/Mesh.hpp>
#include <world/entity/BT_EntityTriangleMesh.hpp>
#include "modules/MeshInstance.hpp"
#include "modules/ModelInstance.hpp"
#include "utils/GameUtils.hpp"
#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include "BulletCollision/CollisionShapes/btTriangleInfoMap.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"

#include <iostream>

using namespace syng;

BT_EntityTriangleMesh::BT_EntityTriangleMesh(BT_World* world, ModelInstance* model) 
    : BT_Entity(world), meshes(model->getMeshInstances()->asMap()), coords(model->getTransform()) {}

BT_EntityTriangleMesh::BT_EntityTriangleMesh(BT_World* world, std::unordered_map<std::string, MeshInstance*> meshes) 
    : BT_Entity(world), meshes(meshes) {}

BT_EntityTriangleMesh::BT_EntityTriangleMesh(BT_World* world, MeshInstance* mesh) 
    : BT_Entity(world), meshes({{"", mesh}}) {}

BT_EntityTriangleMesh::~BT_EntityTriangleMesh() {
    worldAsBT()->getDynamics()->removeRigidBody(body);
    delete body;
    delete shape;
    delete triangleInfoMap;
    delete triangleMesh;
}

const glm::mat4 BT_EntityTriangleMesh::onMotionState() {
    return coords.getTransform();
}

void BT_EntityTriangleMesh::load(bool useQuantizedAabbCompression) {
    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        if (!instance->getMesh()->loaded) {
            std::cerr << "ERROR::Entity::<UNLOADED_MESH>" << std::endl;
            return;
        }
    }
    triangleMesh = new btTriangleMesh();
    triangleInfoMap = new btTriangleInfoMap();

    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        Mesh *mesh = instance->getMesh();
    
        glm::mat4 transform = mesh->getParentToNodeTransform();

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

    worldAsBT()->getDynamics()->addRigidBody(body);
}