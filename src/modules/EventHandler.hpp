#pragma once

#include "SDL3/SDL_events.h"

class SDL_EventHandler
{
public:
    virtual ~SDL_EventHandler() = default;

    virtual void onEvent(const SDL_Event& event) = 0;
};