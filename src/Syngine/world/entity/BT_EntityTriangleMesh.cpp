#include "BT_EntityTriangleMesh.hpp"

#ifdef USE_BULLET

#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <LinearMath/btVector3.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

#include <vector>

using namespace syng;

BT_EntityTriangleMesh::BT_EntityTriangleMesh(ModelInstance* model) 
    : rootNode(model->getRoot()) {}

BT_EntityTriangleMesh::BT_EntityTriangleMesh(MeshInstance* mesh) 
    : rootNode(mesh) {}

BT_EntityTriangleMesh::~BT_EntityTriangleMesh() {
    delete body;
    delete shape;
    delete triangleInfoMap;
    delete triangleMesh;
}

const glm::mat4 BT_EntityTriangleMesh::onMotionState() {
    return coords.getTransform();
}

void BT_EntityTriangleMesh::load(bool useQuantizedAabbCompression) {
    if (body) return;
    pushLeafParents(rootNode, {});

    triangleMesh = new btTriangleMesh();
    triangleInfoMap = new btTriangleInfoMap();

    auto emplaceFunc = [&](int materialId, glm::mat4 transform, std::vector<Vertex>& vertices, std::vector<GLuint>& indices) {
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
            triangleMaterials.push_back(materialId);
        }
    };

    for (auto pair : leafParents) {
        MeshInstance *leaf = pair.first;
        glm::mat4 transform = getWorldTransform(leaf);

        for (auto nMesh : leaf->getMeshes()) {
            Mesh *mesh = nMesh->mesh;

            int materialID = mesh->getMaterial()->getID();
            auto vertices = mesh->getVertices();
            auto indices = mesh->getIndices();

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
                triangleMaterials.push_back(materialID);
            }
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
    body->setUserPointer(this);

    purgeLeafParents();
}

int BT_EntityTriangleMesh::getTrigMaterial(int trigIdx) const {
    return triangleMaterials[trigIdx];
}

#endif