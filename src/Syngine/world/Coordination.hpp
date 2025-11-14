#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/common.hpp>
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

class Coordination2D {
protected:
    glm::mat3 transform = glm::mat3(1.0f);
    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f; // in radians
    glm::vec2 scale = glm::vec2(1.0f);

    virtual void updateTransform();
protected:
    virtual void decompose(const glm::mat3& m);
    virtual void decompose(const glm::mat4& m);
public:
    Coordination2D(glm::vec2 position = glm::vec2(0.0f), float rotation = 0.0f, glm::vec2 scale = glm::vec2(1.0f));

    const glm::mat3& getTransform() const;
    glm::mat4 getTransform4Cpy() const;

    glm::vec2 getPosition() const;
    glm::vec2 getScale() const;
    float getRotation() const;
    glm::vec2 getOrigin() const;

    virtual void setTransform(const glm::mat4& transform);
    virtual void setTransform(const glm::mat3& transform);
    virtual void setPosition(const glm::vec2& pos);
    virtual void addPosition(const glm::vec2& pos);
    virtual void setScale(const glm::vec2& scl);
    virtual void setRotation(float rot);
    virtual void setOrigin(const glm::vec2& org);
};

class Coordination {
protected:
    glm::mat4 transform = glm::mat4(1.0f);
    glm::vec3 origin = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat orientation = glm::quat(1, 0, 0, 0);
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;

    virtual void updateTransform();
    virtual void updateOrientation();
    virtual void updateEulerFromQuat();

    virtual void decompose(const glm::mat4& mat);
public:
    Coordination() = default;
    Coordination(const glm::mat4& transform);
    Coordination(const glm::mat4& transform, const glm::vec3& origin);
    Coordination(const glm::vec3& position, const glm::quat& orientation);
    Coordination(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up);

    const glm::mat4& getTransform() const;
    glm::vec3 getPosition() const;
    glm::vec3 getOrigin() const;
    glm::vec3 getScale() const;
    glm::quat getOrientation() const;
    glm::vec3 getDirection() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    float getPitch() const;
    float getYaw() const;
    float getRoll() const;

    virtual void setTransform(const glm::mat4& mat);
    virtual void setOrigin(const glm::vec3& o);
    virtual void setPosition(const glm::vec3& pos);
    virtual void addPosition(const glm::vec3& delta);
    virtual void setScale(const glm::vec3& s);
    virtual void setOrientation(const glm::quat& q);

    virtual void setDirection(const glm::vec3& newDir, const glm::vec3& upHint = glm::vec3(0, 1, 0));
    virtual void setUp(const glm::vec3& newUp);
    virtual void setRight(const glm::vec3& newRight);

    virtual void setEuler(float yawDeg, float pitchDeg, float rollDeg);
    virtual void setYaw(float degrees);
    virtual void setPitch(float degrees);
    virtual void setRoll(float degrees);
};

}