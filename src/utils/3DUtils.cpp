#include <KEngine/utils/3DUtils.hpp>

glm::vec3 _3Dutils::directionOf(float yaw, float pitch)
{
    return glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
}