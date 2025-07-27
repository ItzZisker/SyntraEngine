#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include "engine/RenderTable.hpp"
#include "modules/Shader.hpp"
#include "Syngine.hpp"

#include <iostream>
#include <math.h>
#include <ostream>

using namespace syng;

GameWindow::GameWindow(std::string title, int initialWidth, int initialHeight) {
    this->title = title;
    this->width = initialWidth;
    this->height = initialHeight;

    attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    attrib(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    attrib(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    addRenderTask([](GameWindow *window) {
        static Uint64 previousCounter = SDL_GetPerformanceCounter();
        Uint64 currentCounter = SDL_GetPerformanceCounter();

        double deltaTime = (double)(currentCounter - previousCounter) / (double) SDL_GetPerformanceFrequency();
        previousCounter = currentCounter;

        window->lastFrameTime = deltaTime;
    });
    addRenderTask([](GameWindow *window) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    });
    addRenderTask([](GameWindow *window) {
        window->windowRenderTable->forEach([window](const std::string& name, WindowRenderable* renderable) {
            renderable->render(window);
        });
    });
}

bool GameWindow::isInitialized() {
    return initialized;
}

void GameWindow::attrib(SDL_GLAttr attr, int value) {
    if (initialized)
        SDL_GL_SetAttribute(attr, value);
    else
        window_attributes.insert({attr, value});
}

int GameWindow::initLoop() {
    if (SDL_Init(SDL_INIT_VIDEO) <= 0) {
        std::cerr << "Syngine: Failed to initialize SDL3: " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_Window *sdlWindow = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (sdlWindow == NULL) {
        std::cerr << "Syngine: Failed to create SDL3 window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -2;
    }
    sdlWindowPtr = sdlWindow;
    glContext =  SDL_GL_CreateContext(sdlWindow);

    SDL_GL_MakeCurrent(sdlWindow, glContext);

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cerr << "Syngine: Failed to initialize GLAD" << std::endl;
        return -3;
    }
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialized = true;

    onCreate(width, height, false);
    for (auto& task : initTasks) {
        task(this);
    }
    while (initialized) {
        for (auto& task : renderTasks) {
            task(this);
        }
        SDL_GL_SwapWindow(sdlWindow);
        lastFrameEvents.clear();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            lastFrameEvents.push_back(event);
            for (auto& handler : eventHandlers) {
                handler->onEvent(event);
            }
            if (event.type == SDL_EVENT_QUIT) {
                closeWindow();
            }
        }
    }
    SDL_DestroyWindow(sdlWindow);
    SDL_GL_DestroyContext(glContext);
    return 0;
}

void GameWindow::addEventHandler(SDL_EventHandler *handler) {
    this->eventHandlers.push_back(handler);
}

void GameWindow::pullEventHandler(SDL_EventHandler *handler) {
    for (int i = 0; i < this->eventHandlers.size(); i++) {
        if (this->eventHandlers[i] == handler) {
            this->eventHandlers.erase(this->eventHandlers.begin() + i);
            return;
        }
    }
}

void GameWindow::forEachFrameEvents(std::function<void(const SDL_Event event)> func) {
    for (const SDL_Event event : lastFrameEvents) {
        func(event);
    }
}

void GameWindow::addRenderTask(std::function<void(GameWindow *)> task) {
    this->renderTasks.push_back(task);
}

void GameWindow::addInitTask(std::function<void(GameWindow *)> task) {
    this->initTasks.push_back(task);
}

void GameWindow::closeWindow() {
    initialized = false;
}

int GameWindow::getGLADLoadStatus() {
    return this->gladLoadStatus;
}

int GameWindow::getSDLWindowStatus() {
    return this->sdlWindowStatus;
}

int GameWindow::getGLFWWindowStatus() {
    return this->glfwWindowStatus;
}

double GameWindow::getLastFrameTime() {
    return this->lastFrameTime;
}

std::vector<SDL_Event> GameWindow::getLastFrameEvents() {
    return this->lastFrameEvents;
}

SDL_Window *GameWindow::getSDLWindowPtr() {
    return sdlWindowPtr;
}

SDL_GLContext GameWindow::getGLContext() {
    return glContext;
}

RenderTable<WindowRenderable>* GameWindow::getWindowRenderTable() {
    return this->windowRenderTable;
}