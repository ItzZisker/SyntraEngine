#pragma once

#include "glm/fwd.hpp"
#include <engine/RenderTable.hpp>
#include <world/World.hpp>
#include <utils/FastMath.hpp>

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
public:
    Coordination(glm::mat4 transform = glm::mat4(1.0f)) {
        this->transform = transform;
    }

    Coordination(const glm::vec3& position, const glm::vec3& direction, const glm::vec3 up) {
        this->transform = glm::mat4(1.0f);
        setPosition(position);
        setDirection(direction);
        setUp(up);
    }

    virtual const glm::mat4& getTransform() {
        return transform;
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

    virtual glm::vec3 getPosition() const {
        return glm::vec3(transform[3]);
    }

    virtual glm::vec3 getDirection() const {
        return glm::normalize(glm::vec3(-transform[2]));
    }

    virtual void setTransform(const glm::mat4& transform) {
        this->transform = transform;
    }

    virtual void setPosition(const glm::vec3& pos) {
        transform[3] = glm::vec4(pos, 1.0f);
    }

    virtual void setUp(const glm::vec3& newUp) {
        glm::vec3 forward = getDirection();
        glm::vec3 right = glm::normalize(glm::cross(newUp, forward));
        glm::vec3 up = glm::normalize(glm::cross(forward, right));

        glm::mat4 rotation = glm::mat4(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);
        rotation[3] = transform[3];

        transform = rotation;
    }

    virtual void setDirection(const glm::vec3& dir) {
        glm::vec3 forward = glm::normalize(dir);
        glm::vec3 worldUp = glm::vec3(0, 1, 0);
        glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));
        glm::vec3 up = glm::normalize(glm::cross(forward, right));

        glm::mat4 rotation(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);
        rotation[3] = transform[3];

        transform = rotation;
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