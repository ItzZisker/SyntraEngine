#include "Coordination.hpp"

#include "Syngine/modules/Scene.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <vector>

using namespace syng;

FrustumPlane::FrustumPlane(): normal(glm::vec3(0.0f, 1.0f, 0.0f)), distance(0.0f) {}
FrustumPlane::FrustumPlane(const glm::vec3& point, const glm::vec3& normalVec) : normal(glm::normalize(normalVec)), distance(-glm::dot(normal, point)) {}

float FrustumPlane::getSignedDistanceToPlane(const glm::vec3& point) const {
    return glm::dot(normal, point) + distance;
}

AABB::AABB(std::vector<glm::vec3> positions) {        
    glm::vec3 min = positions[0];
    glm::vec3 max = positions[0];

    for (const auto& pos : positions) {
        min = glm::min(min, pos);
        max = glm::max(max, pos);
    }
    center = {(min + max) * 0.5f};
    extents = {max.x - center.x, max.y - center.y, max.z - center.z};
}

AABB::AABB(const glm::vec3& min, const glm::vec3& max) : center{(max + min) * 0.5f}, extents{max.x - center.x, max.y - center.y, max.z - center.z} {}
AABB::AABB(const glm::vec3& inCenter, float iI, float iJ, float iK) : center(inCenter), extents{iI, iJ, iK} {}

bool AABB::isOnOrForwardPlane(const FrustumPlane& plane) const {
    const float r =
        extents.x * std::abs(plane.normal.x) +
        extents.y * std::abs(plane.normal.y) +
        extents.z * std::abs(plane.normal.z);
    return -r <= plane.getSignedDistanceToPlane(center);
}

AABB FrustumDiscardable::getBounding() {
    return this->bounding;
}

FrustumDiscardable::FrustumDiscardable(AABB bounding) : bounding(bounding) {}
FrustumDiscardable::FrustumDiscardable() : bounding(AABB(glm::vec3(1.0f), glm::vec3(1.0f))) {}
FrustumDiscardable::~FrustumDiscardable() = default;

Frustum FrustumDiscardable::createFrustum(Scene_T snapshot) {
    Frustum frustum;

    glm::vec3 cameraPos = snapshot.cameraCoords.getPosition();
    glm::vec3 cameraDir = snapshot.cameraCoords.getDirection();
    glm::vec3 cameraUp = snapshot.cameraCoords.getUp();
    glm::vec3 cameraRight = snapshot.cameraCoords.getRight();

    const float halfVSide = snapshot.zFar * tanf(snapshot.FOV * 0.5f);
    const float halfHSide = halfVSide * snapshot.aspectRatio;
    const glm::vec3 frontMultFar = snapshot.zFar * cameraDir;

    frustum.nearFace = { cameraPos + snapshot.zNear * cameraDir, cameraDir };
    frustum.farFace = { cameraPos + frontMultFar, -cameraDir };
    frustum.rightFace = { cameraPos, glm::cross(frontMultFar - cameraRight * halfHSide, cameraUp) };
    frustum.leftFace = { cameraPos, glm::cross(cameraUp, frontMultFar + cameraRight * halfHSide) };
    frustum.topFace = { cameraPos, glm::cross(cameraRight, frontMultFar - cameraUp * halfVSide) };
    frustum.bottomFace = { cameraPos, glm::cross(frontMultFar + cameraUp * halfVSide, cameraRight) };

    return frustum;
}

Frustum FrustumDiscardable::createFrustum(Scene* scene) {
    return createFrustum(scene->getSnapshot());
}

bool FrustumDiscardable::isInFrustum(const Frustum& camFrustum, const glm::mat4& transform) {
    const glm::vec3 globalCenter(transform * glm::vec4(bounding.center, 1.f));

    const glm::vec3 right = glm::normalize(glm::vec3(transform[0])) * bounding.extents.x;
    const glm::vec3 up = glm::normalize(glm::vec3(transform[1])) * bounding.extents.y;
    const glm::vec3 forward = glm::normalize(glm::vec3(transform[2])) * bounding.extents.z;

    const float newIi = std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, right)) +
        std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, up)) +
        std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, forward));
    const float newIj = std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, right)) +
        std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, up)) +
        std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, forward));
    const float newIk = std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, right)) +
        std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, up)) +
        std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, forward));

    const AABB globalAABB(globalCenter, newIi, newIj, newIk);

    return (
        globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
        globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
        globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
        globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
        globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
        globalAABB.isOnOrForwardPlane(camFrustum.farFace)
    );
};

bool FrustumDiscardable::isInView(Scene_T snapshot, const glm::mat4& transform) {
    return isInFrustum(createFrustum(snapshot), transform);
}

bool FrustumDiscardable::isInView(Scene* scene, const glm::mat4& transform) {
    return isInFrustum(createFrustum(scene), transform);
}

bool FrustumDiscardable::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    return !isInView(snapshot, transform);
}

bool FrustumDiscardable::shouldDiscard(Scene* scene, const glm::mat4& transform) {
    return !isInView(scene, transform);
}

class Coordination2D {
protected:
    glm::mat3 transform = glm::mat3(1.0f);
    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f; // in radians
    glm::vec2 scale = glm::vec2(1.0f);

    void updateTransform() {
        glm::mat3 T = glm::mat3(1.0f);
        T[2] = glm::vec3(position - origin, 1.0f);

        float c = cos(rotation);
        float s = sin(rotation);
        glm::mat3 R = glm::mat3(
            c,  s, 0.0f,
           -s,  c, 0.0f,
            0.0f, 0.0f, 1.0f
        );

        glm::mat3 S = glm::mat3(1.0f);
        S[0][0] = scale.x;
        S[1][1] = scale.y;

        glm::mat3 O = glm::mat3(1.0f);
        O[2] = glm::vec3(-origin, 1.0f);

        this->transform = T * R * S * O;
    }
private:
    void decompose(const glm::mat3& m) {
        this->transform = m;
        this->position = glm::vec2(m[2]);

        glm::vec2 col0 = glm::vec2(m[0]);
        glm::vec2 col1 = glm::vec2(m[1]);

        this->scale.x = glm::length(col0);
        this->scale.y = glm::length(col1);

        if (scale.x != 0) col0 /= scale.x;
        if (scale.y != 0) col1 /= scale.y;

        this->rotation = atan2(col0.y, col0.x);
    }
    void decompose(const glm::mat4& m) {
        glm::mat3 m3;
        m3[0] = glm::vec3(m[0]);
        m3[1] = glm::vec3(m[1]);
        m3[2] = glm::vec3(m[3]);
        decompose(m3);
    }
public:
    Coordination2D(glm::vec2 position = glm::vec2(0.0f), float rotation = 0.0f, glm::vec2 scale = glm::vec2(1.0f)) 
        : position(position), rotation(rotation), scale(scale) {
        updateTransform();
    }

    const glm::mat3& getTransform() const {
        return transform;
    }
    glm::mat4 getTransform4Cpy() const {
        glm::mat4 transform4x4 = glm::mat4(1.0f);
        transform4x4[0] = glm::vec4(transform[0], 0.0f);
        transform4x4[1] = glm::vec4(transform[1], 0.0f);
        transform4x4[2] = glm::vec4(0,0,1,0);
        transform4x4[3] = glm::vec4(transform[2], 1.0f);
        return transform4x4;
    }

    glm::vec2 getPosition() const { return position; }
    glm::vec2 getScale() const { return scale; }
    float getRotation() const { return rotation; }
    glm::vec2 getOrigin() const { return origin; }

    virtual void setTransform(const glm::mat4& transform) { decompose(transform); }
    virtual void setTransform(const glm::mat3& transform) { decompose(transform); }
    virtual void setPosition(const glm::vec2& pos) { position = pos; updateTransform(); }
    virtual void addPosition(const glm::vec2& pos) { position += pos; updateTransform(); }
    virtual void setScale(const glm::vec2& scl) { scale = scl; updateTransform(); }
    virtual void setRotation(float rot) { rotation = rot; updateTransform(); }
    virtual void setOrigin(const glm::vec2& org) { origin = org; updateTransform(); }
};

void Coordination::updateTransform() {
    glm::mat4 R = glm::toMat4(orientation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 O = glm::translate(glm::mat4(1.0f), -origin);
    transform = T * R * S * O;
}

void Coordination::updateOrientation() {
    glm::quat qYaw = glm::angleAxis(glm::radians(yaw),   glm::vec3(0, 1, 0));
    glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
    glm::quat qRoll = glm::angleAxis(glm::radians(roll),  glm::vec3(0, 0, 1));

    orientation = glm::normalize(qYaw * qPitch * qRoll);
    updateTransform();
}

void Coordination::updateEulerFromQuat() {
    glm::vec3 euler = glm::eulerAngles(orientation);
    pitch = glm::degrees(euler.x);
    yaw   = glm::degrees(euler.y);
    roll  = glm::degrees(euler.z);
}

void Coordination::decompose(const glm::mat4& mat) {
    transform = mat;
    position = glm::vec3(mat[3]);
    glm::vec3 right = glm::vec3(mat[0]);
    glm::vec3 up = glm::vec3(mat[1]);
    glm::vec3 forward = -glm::vec3(mat[2]);

    scale.x = glm::length(right);
    scale.y = glm::length(up);
    scale.z = glm::length(forward);

    glm::mat3 rotationMat = glm::mat3(
        glm::normalize(right),
        glm::normalize(up),
        glm::normalize(forward)
    );
    orientation = glm::quat_cast(rotationMat);
}

Coordination::Coordination(const glm::mat4& transform) {
    decompose(transform);
}

Coordination::Coordination(const glm::mat4& transform, const glm::vec3& origin)
    : origin(origin) {
    decompose(transform);
}

Coordination::Coordination(const glm::vec3& position, const glm::quat& orientation)
    : position(position), orientation(orientation) {
    updateTransform();
}

Coordination::Coordination(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) {
    glm::mat3 lookAtMat = glm::mat3(glm::lookAt(position, position + direction, up));
    orientation = glm::quat_cast(glm::transpose(lookAtMat)); // transpose fixes handedness
    this->position = position;
    updateTransform();
}

const glm::mat4& Coordination::getTransform() const {
    return transform;
}

glm::vec3 Coordination::getPosition() const {
    return position;
}

glm::vec3 Coordination::getOrigin() const {
    return origin;
}

glm::vec3 Coordination::getScale() const {
    return scale;
}

glm::quat Coordination::getOrientation() const {
    return orientation;
}

glm::vec3 Coordination::getDirection() const {
    return glm::normalize(orientation * glm::vec3(0, 0, -1));
}

glm::vec3 Coordination::getRight() const {
    return glm::normalize(orientation * glm::vec3(1, 0, 0));
}

glm::vec3 Coordination::getUp() const {
    return glm::normalize(orientation * glm::vec3(0, 1, 0));
}

float Coordination::getPitch() const {
    glm::vec3 dir = getDirection();
    return glm::degrees(asin(dir.y));
}

float Coordination::getYaw() const {
    glm::vec3 dir = getDirection();
    float yaw = glm::degrees(atan2(dir.x, -dir.z));
    if (yaw < 0.0f) yaw += 360.0f;
    return yaw;
}

float Coordination::getRoll() const {
    glm::vec3 up = getUp();
    return glm::degrees(atan2(up.x, up.y));
}

void Coordination::setTransform(const glm::mat4& mat) {
    decompose(mat);
}

void Coordination::setOrigin(const glm::vec3& o) {
    origin = o; updateTransform();
}

void Coordination::setPosition(const glm::vec3& pos) {
    position = pos; updateTransform();
}

void Coordination::addPosition(const glm::vec3& delta) {
    position += delta; updateTransform();
}

void Coordination::setScale(const glm::vec3& s) {
    scale = s; updateTransform();
}

void Coordination::setOrientation(const glm::quat& q) {
    orientation = glm::normalize(q); updateTransform();
}

void Coordination::setDirection(const glm::vec3& newDir, const glm::vec3& upHint) {
    glm::vec3 forward = glm::normalize(newDir);
    glm::vec3 right = glm::normalize(glm::cross(upHint, forward));
    glm::vec3 up = glm::normalize(glm::cross(forward, right));

    glm::mat3 rotationMat(right, up, -forward);
    orientation = glm::normalize(glm::quat_cast(rotationMat));
    updateTransform();
}

void Coordination::setUp(const glm::vec3& newUp) {
    glm::vec3 up = glm::normalize(newUp);
    glm::vec3 forward = getDirection();
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    forward = glm::normalize(glm::cross(right, up));

    glm::mat3 rotationMat(right, up, -forward);
    orientation = glm::normalize(glm::quat_cast(rotationMat));
    updateTransform();
}

void Coordination::setRight(const glm::vec3& newRight) {
    glm::vec3 right = glm::normalize(newRight);
    glm::vec3 up = getUp();
    glm::vec3 forward = glm::normalize(glm::cross(right, up));
    up = glm::normalize(glm::cross(forward, right));

    glm::mat3 rotationMat(right, up, -forward);
    orientation = glm::normalize(glm::quat_cast(rotationMat));
    updateTransform();
}

void Coordination::setEuler(float yawDeg, float pitchDeg, float rollDeg) {
    orientation = glm::quat(glm::radians(glm::vec3(pitchDeg, yawDeg, rollDeg)));
    updateTransform();
}

void Coordination::setYaw(float degrees) {
    this->yaw = GameUtils::normalizeAngleDeg(360.0f - degrees);
    updateOrientation();
}

void Coordination::setPitch(float degrees) {
    this->pitch = GameUtils::normalizeAngleDeg(360.0f - degrees);
    updateOrientation();
}

void Coordination::setRoll(float degrees) {
    this->roll = GameUtils::normalizeAngleDeg(360.0f - degrees);
    updateOrientation();
}