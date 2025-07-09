#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Shader.hpp"
#include <Syngine/Syngine.hpp>

#include <iostream>
#include <math.h>
#include <ostream>

namespace Callbacks {
    void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }
}

GameWindow::GameWindow(std::string title, int initialWidth, int initialHeight) {
    this->title = title;
    this->width = initialWidth;
    this->height = initialHeight;

    withHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    withHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    withHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    withHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    addRenderTask([](GameWindow *window) {
        static double previousTime = glfwGetTime();
        double currentTime = glfwGetTime();

        window->lastFrameTime = currentTime - previousTime;
        previousTime = currentTime;
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

void GameWindow::withHint(int hint, int value) {
    if (initialized)
        glfwWindowHint(hint, value);
    else
        window_hints.insert({hint, value});
}

int GameWindow::initLoop() {
    glfwInit();

    for (auto entry : window_hints) {
        glfwWindowHint(entry.first, entry.second);
    }

    GLFWwindow *glfwWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    glfwWindowPtr = glfwWindow;

    if (glfwWindow == NULL) {
        std::cerr << "Syngine: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Syngine: Failed to initialize GLAD" << std::endl;
        return -2;
    }

    glfwSetFramebufferSizeCallback(glfwWindow, Callbacks::glfw_framebuffer_resize_callback);
    glfwGetWindowSize(glfwWindow, &width, &height);

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initialized = true;
    onCreate(width, height, false);

    for (auto &task : initTasks) {
        task(this);
    }

    while (!glfwWindowShouldClose(glfwWindow) && initialized) {
        for (auto &task : renderTasks) {
            task(this);
        }
        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
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

int GameWindow::getGLFWWindowStatus() {
    return this->glfwWindowStatus;
}

double GameWindow::getLastFrameTime() {
    return this->lastFrameTime;
}

GLFWwindow *GameWindow::getGLFWWindowPtr() {
    return glfwWindowPtr;
}

RenderTable<WindowRenderable>* GameWindow::getWindowRenderTable() {
    return this->windowRenderTable;
}