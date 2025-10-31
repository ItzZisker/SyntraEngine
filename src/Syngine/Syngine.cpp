#include "Syngine.hpp"

#include "Syngine/engine/Concurrency.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/engine/TaskQueue.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Material.hpp"
#include "Syngine/modules/Texture.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include "modules/Material.hpp"

#include <ostream>
#include <iostream>

#include <math.h>
#include <stdexcept>

using namespace syng;

GameWindow::GameWindow(std::string title, WindowSize initialSize) {
    this->title = title;
    this->width = initialSize.width;
    this->height = initialSize.height;

#ifdef __EMSCRIPTEN__
    attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    attrib(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    attrib(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    attrib(SDL_GL_RED_SIZE, 32);
    attrib(SDL_GL_GREEN_SIZE, 32);
    attrib(SDL_GL_BLUE_SIZE, 32);
    attrib(SDL_GL_ALPHA_SIZE, 32);

    attrib(SDL_GL_DEPTH_SIZE, 24);
    attrib(SDL_GL_STENCIL_SIZE, 8);
#else
    attrib(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    attrib(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    attrib(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    attrib(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
#endif

    addInitTask([](GameWindow *window){
        FallbackTexture::Diffuse  = {TCBByPlainColor((uint8_t[4]){255, 255, 255, 255})};
        FallbackTexture::Specular = {TCBByPlainColor((uint8_t[4]){255, 255, 255, 255})};
        FallbackTexture::Normal   = {TCBByPlainColor((uint8_t[4]){128, 128, 255, 255})};
        FallbackTexture::Height   = {TCBByPlainColor((uint8_t[4]){255, 255, 255, 255})};
        FallbackTexture::Rough    = {TCBByPlainColor((uint8_t[4]){128, 128, 128, 255})};
        FallbackTexture::Emissive = {TCBByPlainColor((uint8_t[4]){0, 0, 0, 255})};
        FallbackTexture::AO       = {TCBByPlainColor((uint8_t[4]){255, 255, 255, 255})};
        FallbackTexture::Metal    = {TCBByPlainColor((uint8_t[4]){0, 0, 0, 255})};  
    });
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
    if (!initialized) window_attributes.insert({attr, value});
}

void GameWindow::attribMSAA(int samples) {
    if (!initialized) {
        window_attributes.insert({SDL_GL_MULTISAMPLEBUFFERS, 1});
        window_attributes.insert({SDL_GL_MULTISAMPLESAMPLES, samples});
    }
}

int GameWindow::initLoop() {
    Concurrency::initMainThread();
    if (SDL_Init(SDL_INIT_VIDEO) <= 0) {
        std::cerr << "Syngine: Failed to initialize SDL3: " << SDL_GetError() << std::endl;
        return -2;
    }

    for (auto pair : window_attributes) {
        SDL_GL_SetAttribute((SDL_GLAttr) pair.first, pair.second);
    }

    SDL_Window *sdlWindow = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (sdlWindow == NULL) {
        std::cerr << "Syngine: Failed to create SDL3 window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -3;
    }
    sdlWindowPtr = sdlWindow;
    glContext =  SDL_GL_CreateContext(sdlWindow);

    SDL_GL_MakeCurrent(sdlWindow, glContext);

#ifndef __EMSCRIPTEN__ // skip GLAD — WebGL context already provides all functions.
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        std::cerr << "Syngine: Failed to initialize GLAD" << std::endl;
        return -4;
    }
#endif
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialized = true;

    onCreate(width, height, false);
    for (auto& task : initTasks) task(this);
    while (initialized) {
        TaskQueue::Instance().executeAll();
        for (auto& task : renderTasks) task(this);
        SDL_GL_SwapWindow(sdlWindow);
        lastFrameEvents.clear();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            lastFrameEvents.push_back(event);
            for (auto& handler : eventHandlers) handler->onEvent(event);
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    closeWindow();
                break;
                case SDL_EVENT_WINDOW_RESIZED:
                    int width, height;
                    SDL_Window* current = SDL_GetWindowFromID(event.window.windowID);
                    SDL_GetWindowSize(current, &width, &height);
                    this->width = width;
                    this->height = height;
                break;
            }
        }
    }
    for (auto& task : cleanupTasks) {
        task(this);
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

void GameWindow::addInitTask(std::function<void(GameWindow *)> task) {
    this->initTasks.push_back(task);
}

void GameWindow::addRenderTask(std::function<void(GameWindow *)> task) {
    this->renderTasks.push_back(task);
}

void GameWindow::addCleanupTask(std::function<void(GameWindow *)> task) {
    this->cleanupTasks.push_back(task);
}

void GameWindow::closeWindow() {
    initialized = false;
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

TaskQueue& GameWindow::getGameLoopQueue() {
    return this->gameLoopQueue;
}

RenderTable<WindowRenderable>* GameWindow::getWindowRenderTable() {
    return this->windowRenderTable;
}

WindowSize GameWindow::getSize() {
    int width, height;
    SDL_GetWindowSize(sdlWindowPtr, &width, &height);
    return {width, height};
}