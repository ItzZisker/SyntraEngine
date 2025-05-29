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
    std::cout << "A" << std::endl;
    if (!model->loaded) {
        std::cout << "B" << std::endl;
        std::cerr << "ERROR::Entity::<UNLOADED_MODEL>" << std::endl;
        std::cout << "C" << std::endl;
        return;
    }

    std::cout << "D" << std::endl;
    for (const Mesh& mesh : model->meshes) {
        for (const Vertex& vertex : mesh.vertices) {
            glm::vec3 vec = vertex.Position;
            shape->addPoint(btVector3(vec.x, vec.y, vec.z));
        }
    }   
    std::cout << "E" << std::endl;

    if (enablePolyhedral){
        std::cout << "F" << std::endl;
        shape->initializePolyhedralFeatures();
    }
    std::cout << "G" << std::endl;

    std::cout << "H" << std::endl;
    glm::vec3 modelPos = model->getPosition();
    std::cout << "I" << std::endl;
    btDefaultMotionState* motionState = new btDefaultMotionState(
        btTransform(btQuaternion(0,0,0,1), btVector3(modelPos.x, modelPos.y, modelPos.z))
    );
    std::cout << "J" << std::endl;

    btVector3 inertia(0,0,0);
    std::cout << "K" << std::endl;
    shape->calculateLocalInertia(mass, inertia);
    std::cout << "L" << std::endl;

    std::cout << "M" << std::endl;
    body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, motionState, shape, inertia));
    std::cout << "N" << std::endl;
    if (hasRollingFriction) body->setRollingFriction(rollingFriction);
    std::cout << "O" << std::endl;
    body->setFriction(1.0f);
    std::cout << "P" << std::endl;
    body->setDamping(0.8f, 0.2f);
    std::cout << "Q" << std::endl;

    world->getDynamics()->addRigidBody(body);
    std::cout << "R" << std::endl;
}

void EntityConvexHull::render(GameWindow* window) {
    model->render(window);

    btTransform transform;
    
    body->getMotionState()->getWorldTransform(transform);

    btVector3 pos = transform.getOrigin();
    model->setPosition(glm::vec3(pos.x(), pos.y(), pos.z()));
    
    float yaw, pitch, roll;
    transform.getRotation().getEulerZYX(yaw, pitch, roll);
    model->setEuler(
        glm::degrees(yaw),
        glm::degrees(pitch),
        glm::degrees(roll)
    );
}