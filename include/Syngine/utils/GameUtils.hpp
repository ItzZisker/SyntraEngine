#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace GameUtils
{
    long currentTime();

    void debugGLError();

    void debugGLError(const std::string& comment);

    btVector3 toBulletVector(const glm::vec3& vec);

    btQuaternion getBulletRotationFromTransform(const glm::mat4& transform);

    glm::mat4 fromBulletTransform(const btTransform& transform);

    glm::vec3 directionOf(float yaw, float pitch);

    bool shouldDiscard(ShaderRenderable* renderable, Scene_T snapshot);

    bool shouldDiscard(ShaderRenderable* renderable, Scene* scene);

    void renderDV(ShaderRenderable* renderable, Scene_T snapshot, Shader shader, Screenbuffer screen);

    void renderDV(ShaderRenderable* renderable, Scene* scene, Shader shader, Screenbuffer screen);
}