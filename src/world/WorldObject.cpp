#include <Syngine/world/WorldObject.hpp>

#include "Syngine/modules/Camera.hpp"
#include "glm/fwd.hpp"
#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/world/World.hpp>
#include <Syngine/utils/FastMath.hpp>
#include <Syngine/modules/Scene.hpp>

#include <glm/glm.hpp>
#include <vector>

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
    const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) + extents.z * std::abs(plane.normal.z);
    return -r <= plane.getSignedDistanceToPlane(center);
}

FrustumDiscardable::FrustumDiscardable(AABB bounding) : bounding(bounding) {}

FrustumDiscardable::FrustumDiscardable() : bounding(AABB(glm::vec3(1.0f), glm::vec3(1.0f))) {}

FrustumDiscardable::~FrustumDiscardable() = default;

Frustum FrustumDiscardable::createFrustum(Scene* scene) {
    Frustum frustum;

    Camera* camera = scene->getCamera();

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraDir = camera->getDirection();
    glm::vec3 cameraUp = camera->getUp();
    glm::vec3 cameraRight = camera->getRight();

    const float halfVSide = scene->getZFar() * tanf(scene->getFieldOfViewDegrees() * 0.5f);
    const float halfHSide = halfVSide * scene->getAspectRatio();
    const glm::vec3 frontMultFar = scene->getZFar() * cameraDir;

    frustum.nearFace = { cameraPos + scene->getZNear() * cameraDir, cameraDir };
    frustum.farFace = { cameraPos + frontMultFar, -cameraDir };
    frustum.rightFace = { cameraPos, glm::cross(frontMultFar - cameraRight * halfHSide, cameraUp) };
    frustum.leftFace = { cameraPos, glm::cross(cameraUp, frontMultFar + cameraRight * halfHSide) };
    frustum.topFace = { cameraPos, glm::cross(cameraRight, frontMultFar - cameraUp * halfVSide) };
    frustum.bottomFace = { cameraPos, glm::cross(frontMultFar + cameraUp * halfVSide, cameraRight) };

    return frustum;
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

bool FrustumDiscardable::isInView(Scene* scene, const glm::mat4& transform) {
    return isInFrustum(createFrustum(scene), transform);
}

AABB FrustumDiscardable::getBounding() {
    return this->bounding;
}

WorldObject::WorldObject(World *initialWorld) : world(initialWorld) {}

void WorldObject::setWorld(World *world) {
    this->world = world; 
}

World* WorldObject::getWorld() const { 
    return world;
}