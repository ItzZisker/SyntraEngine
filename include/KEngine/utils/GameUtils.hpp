#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include <glm/glm.hpp>

namespace GameUtils
{
    long currentTime();

    void debugGLError();

    btVector3 toBulletVector(const glm::vec3& vec);

    btQuaternion getBulletRotationFromTransform(const glm::mat4& transform);

    glm::mat4 fromBulletTransform(const btTransform& transform);

    glm::vec3 directionOf(float yaw, float pitch);
}