#include <world/WorldObject.hpp>

#include "glm/fwd.hpp"
#include <engine/RenderTable.hpp>
#include <world/World.hpp>
#include <utils/FastMath.hpp>
#include <modules/Scene.hpp>

#include <glm/glm.hpp>
#include <vector>

syng::FrustumPlane::FrustumPlane(): normal(glm::vec3(0.0f, 1.0f, 0.0f)), distance(0.0f) {}

syng::FrustumPlane::FrustumPlane(const glm::vec3& point, const glm::vec3& normalVec) : normal(glm::normalize(normalVec)), distance(-glm::dot(normal, point)) {}

float syng::FrustumPlane::getSignedDistanceToPlane(const glm::vec3& point) const {
    return glm::dot(normal, point) + distance;
}

syng::AABB::AABB(std::vector<glm::vec3> positions) {        
    glm::vec3 min = positions[0];
    glm::vec3 max = positions[0];

    for (const auto& pos : positions) {
        min = glm::min(min, pos);
        max = glm::max(max, pos);
    }
    center = {(min + max) * 0.5f};
    extents = {max.x - center.x, max.y - center.y, max.z - center.z};
}

syng::AABB::AABB(const glm::vec3& min, const glm::vec3& max) : center{(max + min) * 0.5f}, extents{max.x - center.x, max.y - center.y, max.z - center.z} {}

syng::AABB::AABB(const glm::vec3& inCenter, float iI, float iJ, float iK) : center(inCenter), extents{iI, iJ, iK} {}

bool syng::AABB::isOnOrForwardPlane(const FrustumPlane& plane) const {
    const float r =
        extents.x * std::abs(plane.normal.x) +
        extents.y * std::abs(plane.normal.y) +
        extents.z * std::abs(plane.normal.z);
    return -r <= plane.getSignedDistanceToPlane(center);
}

syng::AABB syng::FrustumDiscardable::getBounding() {
    return this->bounding;
}

syng::FrustumDiscardable::FrustumDiscardable(AABB bounding) : bounding(bounding) {}

syng::FrustumDiscardable::FrustumDiscardable() : bounding(AABB(glm::vec3(1.0f), glm::vec3(1.0f))) {}

syng::FrustumDiscardable::~FrustumDiscardable() = default;

syng::Frustum syng::FrustumDiscardable::createFrustum(Scene_T snapshot) {
    Frustum frustum;

    const float halfVSide = snapshot.zFar * tanf(snapshot.FOV * 0.5f);
    const float halfHSide = halfVSide * snapshot.aspectRatio;
    const glm::vec3 frontMultFar = snapshot.zFar * snapshot.cameraDir;

    frustum.nearFace = { snapshot.cameraPos + snapshot.zNear * snapshot.cameraDir, snapshot.cameraDir };
    frustum.farFace = { snapshot.cameraPos + frontMultFar, -snapshot.cameraDir };
    frustum.rightFace = { snapshot.cameraPos, glm::cross(frontMultFar - snapshot.cameraRight * halfHSide, snapshot.cameraUp) };
    frustum.leftFace = { snapshot.cameraPos, glm::cross(snapshot.cameraUp, frontMultFar + snapshot.cameraRight * halfHSide) };
    frustum.topFace = { snapshot.cameraPos, glm::cross(snapshot.cameraRight, frontMultFar - snapshot.cameraUp * halfVSide) };
    frustum.bottomFace = { snapshot.cameraPos, glm::cross(frontMultFar + snapshot.cameraUp * halfVSide, snapshot.cameraRight) };

    return frustum;
}

syng::Frustum syng::FrustumDiscardable::createFrustum(Scene* scene) {
    return createFrustum(scene->getSnapshot());
}

bool syng::FrustumDiscardable::isInFrustum(const Frustum& camFrustum, const glm::mat4& transform) {
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

bool syng::FrustumDiscardable::isInView(Scene_T snapshot, const glm::mat4& transform) {
    return isInFrustum(createFrustum(snapshot), transform);
}

bool syng::FrustumDiscardable::isInView(Scene* scene, const glm::mat4& transform) {
    return isInFrustum(createFrustum(scene), transform);
}

bool syng::FrustumDiscardable::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    return !isInView(snapshot, transform);
}

bool syng::FrustumDiscardable::shouldDiscard(Scene* scene, const glm::mat4& transform) {
    return !isInView(scene, transform);
}

syng::WorldObject::WorldObject(World *initialWorld) : world(initialWorld) {}

void syng::WorldObject::setWorld(World *world) {
    this->world = world; 
}

syng::World* syng::WorldObject::getWorld() const { 
    return world;
}