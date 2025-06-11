#pragma once

#define _USE_MATH_DEFINES
#define _GNU_SOURCE

#include <Syngine/engine/RenderTable.hpp>
#include <Syngine/modules/Shader.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <functional>
#include <unordered_map>
#include <vector>

class GameWindow {
private:
    GLFWwindow *glfwWindowPtr;
    RenderTable<WindowRenderable> *windowRenderTable = new RenderTable<WindowRenderable>(); // Objects that being rendered by window

    double lastFrameTime;

    std::unordered_map<int, int> window_hints;
    std::vector<std::function<void(GameWindow *)>> initTasks, renderTasks;

    std::string title;
    int width, height;

    int glfwWindowStatus, gladLoadStatus;
    bool initialized, disposed;

public:
    GameWindow(std::string title, int width, int height);

    bool isInitialized();

    void withHint(int hint, int value);

    int initLoop();

    void addRenderTask(std::function<void(GameWindow *)> task);

    void addInitTask(std::function<void(GameWindow *)> task);

    void closeWindow();

    int getGLADLoadStatus();

    int getGLFWWindowStatus();

    double getLastFrameTime();

    RenderTable<WindowRenderable> *getWindowRenderTable();

    GLFWwindow *getGLFWWindowPtr();

    int getWindowHeight();

    int getWindowWidth();
};