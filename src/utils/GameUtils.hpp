#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"

#include <glm/glm.hpp>

#include <string>

namespace syng::GameUtils
{
    long currentTime();

    void debugGLError();

    void debugGLError(const std::string& comment);

    bool str_contains(const std::string& target, const std::string& value);

    btVector3 toBulletVector(const glm::vec3& vec);

    btQuaternion getBulletRotationFromTransform(const glm::mat4& transform);

    glm::mat4 fromBulletTransform(const btTransform& transform);

    glm::vec3 directionOf(float yaw, float pitch);

    bool shouldDiscard(ShaderRenderable* renderable, Scene_T snapshot);

    bool shouldDiscard(ShaderRenderable* renderable, Scene* scene);

    void renderDV(ShaderRenderable* renderable, Scene_T snapshot, Shader shader, Screenbuffer screen);

    void renderDV(ShaderRenderable* renderable, Scene* scene, Shader shader, Screenbuffer screen);
}