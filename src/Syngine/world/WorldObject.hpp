#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/World.hpp"
#include "Syngine/utils/FastMath.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace syng
{
struct Scene_T;
class Scene;

struct FrustumPlane {
    glm::vec3 normal;
    float distance;

    FrustumPlane();
    FrustumPlane(const glm::vec3& point, const glm::vec3& normalVec);

    float getSignedDistanceToPlane(const glm::vec3& point) const;
};

struct Frustum {
    FrustumPlane topFace, bottomFace, rightFace, leftFace, farFace, nearFace;
};

class AABB {
public:
    glm::vec3 center {0.0f, 0.0f, 0.0f};
    glm::vec3 extents {0.0f, 0.0f, 0.0f};

    AABB(std::vector<glm::vec3> positions);
    AABB(const glm::vec3& min, const glm::vec3& max);
    AABB(const glm::vec3& inCenter, float iI, float iJ, float iK);

    bool isOnOrForwardPlane(const FrustumPlane& plane) const;
};

class Discardable {
public:
    virtual bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) = 0;
};

class FrustumDiscardable : public Discardable {
protected:
    AABB bounding;
public:
    FrustumDiscardable(AABB bounding);
    FrustumDiscardable();
    ~FrustumDiscardable();

    Frustum createFrustum(Scene* scene);
    Frustum createFrustum(Scene_T snapshot);

    bool isInFrustum(const Frustum& frustum, const glm::mat4& transform);
    bool isInView(Scene_T snapshot, const glm::mat4& transform);
    bool isInView(Scene* scene, const glm::mat4& transform);

    bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) override;
    bool shouldDiscard(Scene* scene, const glm::mat4& transform);

    AABB getBounding();
};

class Coordination {
protected:
    glm::mat4 transform = glm::mat4(1.0f);
    glm::vec3 origin = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    void updateTransform() {
        glm::vec3 forward = glm::normalize(direction);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));
        glm::vec3 correctedUp = glm::normalize(glm::cross(right, forward));

        glm::mat4 rotation(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(correctedUp, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);
        rotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 O = glm::translate(glm::mat4(1.0f), -origin);

        this->transform = T * rotation * S * O;
    }
private:
    void decompose(const glm::mat4& transform) {
        this->transform = transform;

        glm::vec3 position = glm::vec3(transform[3]);
        glm::vec3 right = glm::vec3(transform[0]);
        glm::vec3 upVec = glm::vec3(transform[1]);
        glm::vec3 forward = -glm::vec3(transform[2]);

        this->scale.x = glm::length(right);
        this->scale.y = glm::length(upVec);
        this->scale.z = glm::length(forward);

        this->position = position;
        this->direction = glm::normalize(forward);
        this->up = glm::normalize(upVec);

        updateTransform();
    }
public:
    Coordination(glm::mat4 transform = glm::mat4(1.0f)) {
        decompose(transform);
    }

    Coordination(const glm::mat4& transform, const glm::vec3& origin) : origin(origin) {
        decompose(transform);
    }

    Coordination(const glm::vec3& position, const glm::vec3& direction, const glm::vec3 up) : position(position), direction(direction), up(up), scale(1.0f) {
        updateTransform();
    }

    virtual float getYaw() const {
        glm::vec3 dir = getDirection();
        return glm::degrees(atan2(dir.x, -dir.z));
    }

    virtual float getPitch() const {
        return glm::degrees(asin(getDirection().y));
    }

    virtual glm::vec3 getUp() const {
        return glm::normalize(glm::vec3(transform[1]));
    }

    virtual glm::vec3 getRight() const {
        return glm::normalize(glm::cross(getDirection(), getUp()));
    }

    virtual glm::vec3 getDirection() const {
        return glm::normalize(glm::vec3(-transform[2]));
    }

    virtual glm::vec3 getPosition() const {
        return this->position;
    }

    virtual glm::vec3 getScale() const {
        return scale;
    }

    virtual const glm::mat4& getTransform() {
        return this->transform;
    }

    virtual glm::vec3 getOrigin() const {
        return this->origin;
    }

    virtual void setTransform(const glm::mat4& transform) {
        decompose(transform);
    }

    virtual void setOrigin(const glm::vec3& newOrigin) {
        this->origin = newOrigin;
        updateTransform();
    }

    virtual void setPosition(const glm::vec3& pos) {
        this->position = pos;
        updateTransform();
    }

    virtual void addPosition(const glm::vec3& pos) {
        this->position += pos;
        updateTransform();
    }

    virtual void setUp(const glm::vec3& up) {
        this->up = up;
        updateTransform();
    }

    virtual void setDirection(const glm::vec3& dir) {
        this->direction = glm::normalize(dir);
        updateTransform();
    }

    virtual void setScale(const glm::vec3& scale) {
        this->scale = scale;
        updateTransform();
    }
};

class WorldObject : public Coordination {
protected:
    World *world;
public:
    WorldObject(World *initialWorld);

    void setWorld(World *world);

    World *getWorld() const;
};
}