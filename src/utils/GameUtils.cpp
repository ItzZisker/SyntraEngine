#include <KEngine/utils/GameUtils.hpp>

btVector3 GameUtils::toBulletVector(glm::vec3 vec) {
    return btVector3(vec[0], vec[1], vec[2]);
}

glm::vec3 GameUtils::directionOf(float yaw, float pitch) {
    return glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    );
}