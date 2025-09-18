#include "BT_EntityConvexHull.hpp"
#ifdef USE_BULLET

#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/world/World.hpp"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <LinearMath/btVector3.h>

using namespace syng;

BT_EntityConvexHull::BT_EntityConvexHull(BT_World* world, float mass, MeshInstance* mesh) 
    : mass(mass), meshes({{"ROOT", mesh}}) {}

BT_EntityConvexHull::BT_EntityConvexHull(BT_World* world, float mass, ModelInstance* model) 
    : mass(mass), meshes(model->getMeshInstances()->asMap()) {}

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

    int numPoints = 0;
    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        if (instance->getSelf()) {
            numPoints += instance->getSelf()->getVertices().size();
        } else {
            instance->getChildren()->forEach([&](const std::string& key, MeshInstance *child){
                numPoints += child->getSelf()->getVertices().size();
            });
        }
    }

    float* points = new float[3 * numPoints];
    int i = 0;

    auto emplaceFunc = [&](glm::mat4 transform, std::vector<Vertex>& vertices) {
        for (const Vertex& vertex : vertices) {
            glm::vec4 vec = glm::vec4(vertex.position, 1.0f) * transform;
            points[i++] = vec[0];
            points[i++] = vec[1];
            points[i++] = vec[2];
        }
    };

    for (const auto& it : meshes) {
        MeshInstance* instance = it.second;
        if (instance->getSelf()) {
            emplaceFunc(instance->getTransform(), instance->getSelf()->getVertices());
        } else {
            instance->getChildren()->forEach([&](const std::string& key, MeshInstance *child){
                emplaceFunc(instance->getTransform() * child->getTransform(), child->getSelf()->getVertices());
            });
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
}
#endif