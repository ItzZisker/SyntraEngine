#pragma once

#include "SDL3/SDL_events.h"

#include <vector>

namespace syng
{
class SDL_EventHandler
{
public:
    virtual ~SDL_EventHandler() = default;

    virtual void onEvent(const SDL_Event& event) = 0;
    virtual void onEvents(std::vector<SDL_Event> events) {
        for (auto& event : events) {
            onEvent(event);
        }
    }
};
}