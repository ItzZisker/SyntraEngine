#pragma once

#define _USE_MATH_DEFINES
#define _GNU_SOURCE

#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/modules/Shader.hpp>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <functional>
#include <unordered_map>
#include <vector>

class GameWindow {
private:
    GLFWwindow *glfwWindowPtr;

    Shader batchShader = Shader("shaders/batchVertex.glsl", "shaders/batchFrag.glsl");
    RenderTable *renderTable = new RenderTable();

    double lastFrameTime;

    std::unordered_map<int, int> window_hints;

    std::vector<std::function<void(GameWindow *)>> initTasks;
    std::vector<std::function<void(GameWindow *)>> renderTasks;

    std::string title;
    int width, height;

    int glfwWindowStatus, gladLoadStatus;
    bool initialized, disposed;

public:
    GameWindow(std::string title, int width, int height);

    bool isInitialized();

    void withHint(int hint, int value);

    int initWindow();

    void addRenderTask(std::function<void(GameWindow *)> task);

    void addInitTask(std::function<void(GameWindow *)> task);

    void closeWindow();

    int getGLADLoadStatus();

    int getGLFWWindowStatus();

    double getLastFrameTime();

    RenderTable *getRenderTable();

    GLFWwindow *getGLFWWindowPtr();

    Shader getBatchShader();

    int getWindowHeight();

    int getWindowWidth();
};