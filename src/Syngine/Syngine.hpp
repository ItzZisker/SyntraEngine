#pragma once

#include "platform/GLSupport.hpp"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "Syngine/engine/TaskQueue.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/EventHandler.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/ports/GLPort.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace syng
{

#ifdef __EMSCRIPTEN__
inline void SG_LOG(const std::string& msg) {
    EM_ASM({ console.log("[SG_WEBGL] " + UTF8ToString($0)); }, msg.c_str());
}
#else
inline void SG_LOG(const std::string& msg) {}
#endif

struct WindowSize {
    int width, height;
};

class GameWindow : public Screenbuffer {
private:
	SDL_Window *sdlWindowPtr; 
    SDL_GLContext glContext;

    GLSupport support;
    TaskQueue gameLoopQueue;
    RenderTable<WindowRenderable> *windowRenderTable = new RenderTable<WindowRenderable>(); // Objects that being rendered by window

    double lastFrameTime;

    std::unordered_map<int, int> window_attributes;
    std::vector<std::function<void(GameWindow *)>> initTasks, renderTasks, cleanupTasks;
    
    std::vector<SDL_EventHandler*> eventHandlers;
    std::vector<SDL_Event> lastFrameEvents;

    std::string title;

    bool initialized, disposed;
public:
    GameWindow(std::string title, WindowSize initialSize);

    // Before GL Init
    bool isInitialized();
    void attrib(SDL_GLAttr attr, int value);
    void attribMSAA(int samples = 4);
    int initLoop();

    // After GL Init
    void updateScreenBufferSize();
    void addEventHandler(SDL_EventHandler *handler);
    void pullEventHandler(SDL_EventHandler *handler);
    void forEachFrameEvents(std::function<void(const SDL_Event event)> func);
    void closeWindow();

    const GLSupport& getGLSupport();
    double getLastFrameTime();
    std::vector<SDL_Event> getLastFrameEvents();
    TaskQueue& getGameLoopQueue();
    RenderTable<WindowRenderable> *getWindowRenderTable();
	SDL_Window *getSDLWindowPtr();
    SDL_GLContext getGLContext();
    WindowSize getSize();

    // Both
    void addInitTask(std::function<void(GameWindow *)> task);
    void addRenderTask(std::function<void(GameWindow *)> task);
    void addCleanupTask(std::function<void(GameWindow *)> task);
};
}