#pragma once

#include "glm/fwd.hpp"
#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/world/World.hpp>
#include <Syngine/utils/GameUtils.hpp>
#include <Syngine/utils/FastMath.hpp>

#include <glm/glm.hpp>
#include <vector>

struct FrustumPlane {
    glm::vec3 normal;
    float distance;
};

class BoundingBox {
public:
    glm::vec3 min;
    glm::vec3 max;

    BoundingBox(std::vector<glm::vec3> positions) {        
        this->min = positions[0];
        this->max = positions[0];

        for (const auto& pos : positions) {
            this->min = glm::min(min, pos);
            this->max = glm::max(max, pos);
        }
    }

    BoundingBox(glm::vec3 min, glm::vec3 max) {
        this->min = min;
        this->max = max;
    }
};

class DiscardableObject {
protected:
    BoundingBox AABB;
public:
    DiscardableObject(BoundingBox AABB) : AABB(AABB) {}

    DiscardableObject() : AABB(BoundingBox(glm::vec3(1.0f), glm::vec3(1.0f))) {}

    ~DiscardableObject() = default;

    std::vector<FrustumPlane> extractFrustumPlanes(const glm::mat4& viewProj) {            
        std::vector<FrustumPlane> planes(6);

        glm::vec4 rowX = viewProj[0];
        glm::vec4 rowY = viewProj[1];
        glm::vec4 rowZ = viewProj[2];
        glm::vec4 rowW = viewProj[3];

        planes[0] = { glm::vec3(rowW + rowX), (rowW + rowX).w };
        planes[1] = { glm::vec3(rowW - rowX), (rowW - rowX).w };
        planes[2] = { glm::vec3(rowW + rowY), (rowW + rowY).w };
        planes[3] = { glm::vec3(rowW - rowY), (rowW - rowY).w };
        planes[4] = { glm::vec3(rowW + rowZ), (rowW + rowZ).w };
        planes[5] = { glm::vec3(rowW - rowZ), (rowW - rowZ).w };

        for (auto& plane : planes) { // normalize
            float divisor = FastMath::inv_sqrt(FastMath::glmLen2(plane.normal));
            plane.normal *= divisor;
            plane.distance *= divisor;
        }
        return planes;
    }

    bool isInFrustum(const std::vector<FrustumPlane>& planes, const glm::mat4& transform) {        
        for (const auto& plane : planes) {
            glm::vec3 corners[8];

            corners[0] = glm::vec3(transform * glm::vec4(AABB.min.x, AABB.min.y, AABB.min.z, 1.0));
            corners[1] = glm::vec3(transform * glm::vec4(AABB.max.x, AABB.min.y, AABB.min.z, 1.0));
            corners[2] = glm::vec3(transform * glm::vec4(AABB.min.x, AABB.max.y, AABB.min.z, 1.0));
            corners[3] = glm::vec3(transform * glm::vec4(AABB.min.x, AABB.min.y, AABB.max.z, 1.0));
            corners[4] = glm::vec3(transform * glm::vec4(AABB.max.x, AABB.max.y, AABB.min.z, 1.0));
            corners[5] = glm::vec3(transform * glm::vec4(AABB.max.x, AABB.min.y, AABB.max.z, 1.0));
            corners[6] = glm::vec3(transform * glm::vec4(AABB.min.x, AABB.max.y, AABB.max.z, 1.0));
            corners[7] = glm::vec3(transform * glm::vec4(AABB.max.x, AABB.max.y, AABB.max.z, 1.0));

            bool allOutside = true;

            for (int i = 0; i < 8; ++i) {
                if (glm::dot(plane.normal, corners[i]) + plane.distance >= 0) {
                    allOutside = false;
                    break;
                }
            }
            return allOutside;
        }
        return true;
    }

    bool isInView(const glm::mat4 projection, const glm::mat4& transform) {        
        return isInFrustum(extractFrustumPlanes(projection), transform);
    }

    bool shouldDiscard(const glm::mat4 projection, const glm::mat4& transform) {
        return !isInView(projection, transform);
    }

    BoundingBox getAABB() {
        return AABB;
    }
};

class CoordinatedObject {
protected:
    glm::mat4 transform = glm::mat4(1.0f);
public:
    CoordinatedObject(glm::mat4 transform = glm::mat4(1.0f)) {
        this->transform = transform;
    }

    virtual const glm::mat4& getTransform() {
        return transform;
    }

    virtual glm::vec3 getPosition() const {
        return glm::vec3(transform[3]);
    }

    virtual glm::vec3 getDirection() const {
        return glm::normalize(glm::vec3(-transform[2]));
    }

    virtual float getYaw() const {
        glm::vec3 dir = getDirection();
        return glm::degrees(atan2(dir.x, -dir.z));
    }

    virtual float getPitch() const {
        return glm::degrees(asin(getDirection().y));
    }

    virtual void setTransform(const glm::mat4& transform) {
        this->transform = transform;
    }

    virtual void setPosition(const glm::vec3& pos) {
        transform[3] = glm::vec4(pos, 1.0f);
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

class WorldObject : public CoordinatedObject {
protected:
    World *world;
public:
    WorldObject(World *initialWorld) : world(initialWorld) {}

    void setWorld(World *world) {
        this->world = world; 
    }

    World *getWorld() const { 
        return world;
    }
};