#include "SampleCallbacks.hpp"

#include "SDL3/SDL_events.h"
#include "Syngine.hpp"
#include "modules/Camera.hpp"
#include "SampleGame.hpp"
#include "utils/GameUtils.hpp"

SampleMouseEventHandler::SampleMouseEventHandler(SampleGame *game) : game(game) {}
 
void SampleMouseEventHandler::onEvent(const SDL_Event& event) {
    if (!game->mouseCaptured || event.type != SDL_EVENT_MOUSE_MOTION) {
		return;
	}
    float x = static_cast<float>(event.motion.x);
    float y = static_cast<float>(event.motion.y);
    float xrel = static_cast<float>(event.motion.xrel);
    float yrel = static_cast<float>(event.motion.yrel);

    xrel *= sensitivity;
    yrel *= sensitivity;
    game->yaw += xrel;
    game->pitch += yrel;
    game->yaw = fmod(game->yaw, 360.0f);
    game->pitch = glm::clamp(game->pitch, -89.0f, 89.0f);
    game->camera->setDirection(GameUtils::directionOf(game->yaw, -game->pitch));
}

SampleKeyHandler::SampleKeyHandler(SampleGame *game) : game(game) {}

void SampleKeyHandler::onKeysState(double lastFrameTime, const bool* state) {
    const float cameraSpeed = 1.5f * lastFrameTime;

    glm::vec3 horizontalDirection(0.0f);

    horizontalDirection.x = cos(glm::radians(game->yaw));
    horizontalDirection.z = sin(glm::radians(game->yaw));

    glm::vec3 cameraPos = game->camera->getPosition();
    glm::vec3 cameraUp = game->camera->getUp();

    glm::vec3 moving(0.0f);

    if (state[SDL_SCANCODE_UP]) {
        glm::vec3 dir(1.0f, 0.0f, 0.0f);
        dir *= (lastFrameTime * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        glm::vec3 dir(-1.0f, 0.0f, 0.0f);
        dir *= (lastFrameTime * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        glm::vec3 dir(0.0f, 0.0f, -1.0f);
        dir *= (lastFrameTime * moveAccel);
        moving += dir;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        glm::vec3 dir(0.0f, 0.0f, 1.0f);
        dir *= (lastFrameTime * moveAccel);
        moving += dir;
    }

    btRigidBody* body = game->appleEntity->getBody();

    body->activate();
    if (body->getLinearVelocity().length2() <= 3 && moving.length() > 0.0f) {
        moving *= (3.0f / moving.length());
    }
    body->applyCentralImpulse(GameUtils::toBulletVector(moving));

    if (state[SDL_SCANCODE_W])
        cameraPos += cameraSpeed * horizontalDirection;
    if (state[SDL_SCANCODE_S])
        cameraPos -= cameraSpeed * horizontalDirection;
    if (state[SDL_SCANCODE_A])
        cameraPos -= glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (state[SDL_SCANCODE_D])
        cameraPos += glm::normalize(glm::cross(horizontalDirection, cameraUp)) * cameraSpeed;
    if (state[SDL_SCANCODE_SPACE])
        cameraPos += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    if (state[SDL_SCANCODE_LSHIFT])
        cameraPos -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);

    game->camera->setPosition(cameraPos);

    if (state[SDL_SCANCODE_P])
        game->overWorld->paused = false;

    static bool escapePressedLastFrame = false;
    if (state[SDL_SCANCODE_ESCAPE]) {
        if (!escapePressedLastFrame) {
            game->mouseCaptured = !game->mouseCaptured;
            SDL_SetWindowRelativeMouseMode(game->window->getSDLWindowPtr(), game->mouseCaptured);
            game->firstMouse = true;
        }
        escapePressedLastFrame = true;
    } else {
        escapePressedLastFrame = false;
    }
}