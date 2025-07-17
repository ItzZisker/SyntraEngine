#pragma once

#define _USE_MATH_DEFINES
#define _GNU_SOURCE

#include <engine/RenderTable.hpp>
#include <modules/Screenbuffer.hpp>
#include <modules/Shader.hpp>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <functional>
#include <unordered_map>
#include <vector>

class GameWindow : public Screenbuffer {
private:
	SDL_Window * sdlWindowPtr; 
    SDL_GLContext glContext;

    RenderTable<WindowRenderable> *windowRenderTable = new RenderTable<WindowRenderable>(); // Objects that being rendered by window

    double lastFrameTime;

    std::unordered_map<int, int> window_attributes;
    std::vector<std::function<void(GameWindow *)>> initTasks, renderTasks;

    std::string title;
    int width, height;

    int sdlWindowStatus, glfwWindowStatus, gladLoadStatus;
    bool initialized, disposed;

public:
    GameWindow(std::string title, int width, int height);

    bool isInitialized();

    void attrib(SDL_GLAttr attr, int value);

    int initLoop();

    void addRenderTask(std::function<void(GameWindow *)> task);

    void addInitTask(std::function<void(GameWindow *)> task);

    void closeWindow();

    int getGLADLoadStatus();

	int getSDLWindowStatus();

    int getGLFWWindowStatus();

    double getLastFrameTime();

    RenderTable<WindowRenderable> *getWindowRenderTable();

	SDL_Window *getSDLWindowPtr();

    SDL_GLContext getGLContext();
};