#include "GameUtils.hpp"

#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/world/Coordination.hpp"

#include "LinearMath/btTransform.h"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <glad/glad.h>
#include <iostream>
#include <chrono>

#ifdef USE_ASSIMP
#include <assimp/matrix4x4.h>
#endif

using namespace syng;

long GameUtils::currentTime() {
    namespace sc = std::chrono;
    return sc::duration_cast<sc::milliseconds>(
        sc::system_clock::now().time_since_epoch()
    ).count();;
}

void GameUtils::debugGLError() {
    debugGLError("");
}

void GameUtils::debugGLError(const std::string& comment) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        if (comment.empty())
            std::cerr << "GL Error: " << err << std::endl;
        else
            std::cerr << "GL Error: " << err << " (" << comment << ")" << std::endl;
    }
}

void GameUtils::str_replaceAll(std::string& target, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = target.find(from, pos)) != std::string::npos) {
        target.replace(pos, from.length(), to);
        pos += to.length();
    }
}

bool GameUtils::str_contains(const std::string& target, const std::string& value) {
    return target.find(value) != std::string::npos;
}

btVector3 GameUtils::toBulletVector(const glm::vec3& vec) {
    return btVector3(vec[0], vec[1], vec[2]);
}

btQuaternion GameUtils::getBulletRotationFromTransform(const glm::mat4& transform) {
    glm::quat q = glm::quat_cast(glm::mat3(transform));
    return btQuaternion(q.x, q.y, q.z, q.w);
}

#ifdef USE_ASSIMP
glm::mat4 GameUtils::convertToGLMMatrix(const aiMatrix4x4& aiMat) {
    glm::mat4 mat;
    mat[0][0] = aiMat.a1; mat[1][0] = aiMat.a2; mat[2][0] = aiMat.a3; mat[3][0] = aiMat.a4;
    mat[0][1] = aiMat.b1; mat[1][1] = aiMat.b2; mat[2][1] = aiMat.b3; mat[3][1] = aiMat.b4;
    mat[0][2] = aiMat.c1; mat[1][2] = aiMat.c2; mat[2][2] = aiMat.c3; mat[3][2] = aiMat.c4;
    mat[0][3] = aiMat.d1; mat[1][3] = aiMat.d2; mat[2][3] = aiMat.d3; mat[3][3] = aiMat.d4;
    return mat;
}
#endif

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

bool GameUtils::shouldDiscard(ShaderRenderable* renderable, Scene_T snapshot) {
    Discardable* discardable = dynamic_cast<Discardable*>(renderable);
    Coordination* coords = dynamic_cast<Coordination*>(renderable);

    return discardable && coords && discardable->shouldDiscard(snapshot, coords->getTransform());
}

bool GameUtils::shouldDiscard(ShaderRenderable* renderable, Scene* scene) {
    return GameUtils::shouldDiscard(renderable, scene->getSnapshot());
}

void GameUtils::renderDV(ShaderRenderable *renderable, Scene_T snapshot, Shader& shader, Screenbuffer screen) {
    if (GameUtils::shouldDiscard(renderable, snapshot)) {
        return;
    }
    if (ModelInstance* mI = dynamic_cast<ModelInstance*>(renderable)) {
        mI->renderDV(snapshot, shader, screen);
    } else {
        renderable->render(shader, screen);
    }
}

void GameUtils::renderDV(ShaderRenderable *renderable, Scene *scene, Shader& shader, Screenbuffer screen) {
    GameUtils::renderDV(renderable, scene->getSnapshot(), shader, screen);
}