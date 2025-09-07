#pragma once

#include "Syngine/Syngine.hpp"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keyboard.h"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/EventHandler.hpp"

using namespace syng;

class SampleGame;

class SampleMouseEventHandler : public SDL_EventHandler {
private:
    SampleGame *game;
public:
    float sensitivity = 0.1f;
    double lastX, lastY;

    SampleMouseEventHandler(SampleGame *game);

    void onEvent(const SDL_Event& event) override;
};

class SampleKeyHandler : public WindowRenderable {
public:
    float moveAccel = 2.0f;

    SampleKeyHandler(SampleGame *game);

    void render(GameWindow* window) override {
        onKeysState(window->getLastFrameTime(), SDL_GetKeyboardState(NULL));
    }
private:
    SampleGame *game;

    void onKeysState(double lastFrameTime, const bool* state);
};