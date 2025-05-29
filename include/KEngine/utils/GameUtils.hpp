#include "LinearMath/btVector3.h"
#include <glm/glm.hpp>

namespace GameUtils
{
    btVector3 toBulletVector(glm::vec3 vec);

    glm::vec3 directionOf(float yaw, float pitch);
}