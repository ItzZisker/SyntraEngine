#include "BT_EntityConvexHull.hpp"
#ifdef USE_BULLET

#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <LinearMath/btVector3.h>

using namespace syng;

BT_EntityConvexHull::BT_EntityConvexHull(float mass, MeshInstance* meI) 
    : mass(mass), rootNode(meI) {}

BT_EntityConvexHull::BT_EntityConvexHull(float mass, ModelInstance* model) 
    : mass(mass), rootNode(model->getRoot()) {}

BT_EntityConvexHull::~BT_EntityConvexHull() {
    delete body;
    delete shape;
}

const glm::mat4 BT_EntityConvexHull::onMotionState() {
    btTransform transform;
    body->getMotionState()->getWorldTransform(transform);
    return GameUtils::fromBulletTransform(transform);
}

void BT_EntityConvexHull::load(bool enablePolyhedral) {
    pushLeafParents(rootNode, {});

    int numPoints = 0;
    getNumPoints(rootNode, numPoints);

    float* points = new float[3 * numPoints];
    int i = 0;

    for (auto pair : leafParents) {
        MeshInstance *leaf = pair.first;
        glm::mat4 transform = getWorldTransform(leaf);

        for (auto nMesh : leaf->getMeshes()) {
            for (const Vertex& vertex : nMesh->mesh->getVertices()) {
                glm::vec4 vec = glm::vec4(vertex.position, 1.0f) * transform;
                points[i++] = vec[0];
                points[i++] = vec[1];
                points[i++] = vec[2];
            }
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

    purgeLeafParents();
}
#endif