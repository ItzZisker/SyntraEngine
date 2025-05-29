#include "KEngine/world/WorldObject.hpp"
#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/modules/Model.hpp>

#include <btBulletDynamicsCommon.h>
#include <Bullet3Common/b3Vector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>

class EntityTriangleMesh : public WorldObject
{
private:
    Model* model;
    btRigidBody* body;
    btTriangleMesh* triangleMesh;
    btBvhTriangleMeshShape* shape;
    float mass;
public:
    EntityTriangleMesh(World* world, float mass, Model* model);

    ~EntityTriangleMesh();

    void load(bool useQuantiziedAabbCompression = true);

    void render(GameWindow* window) override;

    btRigidBody* getBody() {
        return this->body;
    }

    btBvhTriangleMeshShape* getShape() {
        return this->shape;
    }

    Model* getModel() {
        return this->model;
    }

    World* getWorld() {
        return this->world;
    }
};