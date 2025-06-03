#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"

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
}