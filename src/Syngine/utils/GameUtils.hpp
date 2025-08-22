#pragma once

#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"

#ifdef USE_ASSIMP
#include "assimp/matrix4x4.h"
#endif
#include <glm/glm.hpp>

#include <string>

namespace syng::GameUtils
{
    long currentTime();

    void debugGLError();
    void debugGLError(const std::string& comment);

    void str_replaceAll(std::string& target, const std::string& from, const std::string& to);
    bool str_contains(const std::string& target, const std::string& value);

    btVector3 toBulletVector(const glm::vec3& vec);
    btQuaternion getBulletRotationFromTransform(const glm::mat4& transform);

#ifdef USE_ASSIMP
    glm::mat4 convertToGLMMatrix(const aiMatrix4x4& aiMat);
#endif
    glm::mat4 fromBulletTransform(const btTransform& transform);
    glm::vec3 directionOf(float yaw, float pitch);

    bool shouldDiscard(ShaderRenderable* renderable, Scene_T snapshot);
    bool shouldDiscard(ShaderRenderable* renderable, Scene* scene);

    void renderDV(ShaderRenderable* renderable, Scene_T snapshot, Shader& shader, Screenbuffer screen);
    void renderDV(ShaderRenderable* renderable, Scene* scene, Shader& shader, Screenbuffer screen);
}