#include "LinearMath/btTransform.h"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <glad/glad.h>
#include <KEngine/utils/GameUtils.hpp>
#include <iostream>
#include <chrono>

long GameUtils::currentTime() {
    namespace sc = std::chrono;
    return sc::duration_cast<sc::milliseconds>(
        sc::system_clock::now().time_since_epoch()
    ).count();;
}

void GameUtils::debugGLError() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "GL Error: " << err << std::endl;
    }
}

btVector3 GameUtils::toBulletVector(const glm::vec3& vec) {
    return btVector3(vec[0], vec[1], vec[2]);
}

btQuaternion GameUtils::getBulletRotationFromTransform(const glm::mat4& transform) {
    glm::quat q = glm::quat_cast(glm::mat3(transform));
    return btQuaternion(q.x, q.y, q.z, q.w);
}

glm::mat4 GameUtils::fromBulletTransform(const btTransform& transform) {
    glm::mat4 result(1.0f);

    const btMatrix3x3& basis = transform.getBasis();
    const btVector3& origin = transform.getOrigin();

    result[0][0] = basis[0][0];
    result[0][1] = basis[1][0];
    result[0][2] = basis[2][0];

    result[1][0] = basis[0][1];
    result[1][1] = basis[1][1];
    result[1][2] = basis[2][1];

    result[2][0] = basis[0][2];
    result[2][1] = basis[1][2];
    result[2][2] = basis[2][2];

    result[3][0] = origin.getX();
    result[3][1] = origin.getY();
    result[3][2] = origin.getZ();

    return result;
}

glm::vec3 GameUtils::directionOf(float yaw, float pitch) {
    return glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    );
}