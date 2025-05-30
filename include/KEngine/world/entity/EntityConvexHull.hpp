#include <KEngine/world/WorldObject.hpp>
#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

class EntityConvexHull : public WorldObject
{
private:
    Model* model;
    btRigidBody* body;
    btConvexHullShape* shape;
    float mass;
public:
    bool hasRollingFriction = true;
    float friction = 1.0f, rollingFriction = 0.3f, linearDamping = 0.8f, angularDamping = 0.2f;

    EntityConvexHull(World* world, float mass, Model* model);

    ~EntityConvexHull();

    void load(bool enablePolyhedral = true);

    void render(GameWindow* window) override;

    btRigidBody* getBody() {
        return this->body;
    }

    btConvexHullShape* getShape() {
        return this->shape;
    }

    Model* getModel() {
        return this->model;
    }

    World* getWorld() {
        return this->world;
    }
};