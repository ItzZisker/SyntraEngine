#pragma once

#define _USE_MATH_DEFINES
#define _GNU_SOURCE

#include <KEngine/engine/RenderTable.hpp>
#include <KEngine/modules/Shader.hpp>
#include <KEngine/utils/3DUtils.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>

// KEngine Window
namespace kwindow
{
    class WindowSize
    {
    public:
        int width, height;

        WindowSize(int width, int height)
        {
            this->width = width;
            this->height = height;
        }
    };

    class GameWindow
    {
    private:
        GLFWwindow *glfwWindowPtr;
        Shader batchShader = Shader("shaders/batch.vs", "shaders/batch.fs");

        double lastFrameTime;

        std::unordered_map<int, int> window_hints;
        RenderTable* renderTable;

        std::vector<std::function<void(kwindow::GameWindow *)>> initTasks;
        std::vector<std::function<void(kwindow::GameWindow *)>> renderTasks;

        std::string title;
        int width, height;

        int glfwWindowStatus, gladLoadStatus;
        bool initialized, disposed;

    public:
        GameWindow(std::string title, int width, int height);

        bool isInitialized();

        void withHint(int hint, int value);

        int initWindow();

        void addRenderTask(std::function<void(kwindow::GameWindow *)> task);

        void addInitTask(std::function<void(kwindow::GameWindow *)> task);

        void closeWindow();

        int getGLADLoadStatus();

        int getGLFWWindowStatus();

        double getLastFrameTime();

        RenderTable* getRenderTable();

        GLFWwindow *getGLFWWindowPtr();

        Shader getBatchShader();

        WindowSize getWindowSize();
    };
}

// KEngine Window Components
namespace kcomp
{
    class Renderable
    {
    public:
        virtual void render(kwindow::GameWindow* window) = 0;

        virtual ~Renderable() = default;
    };

    class World
    {
    public:
        const unsigned int id;
        const std::string name;

        World(unsigned int id, std::string name) : id(id), name(name) {}
    };

    class WorldObject : public kcomp::Renderable
    {
    protected:
        kcomp::World* world;
        glm::vec3 position, direction;

    public:
        WorldObject(kcomp::World* initialWorld) : world(initialWorld) {}

        virtual void render(kwindow::GameWindow* window) = 0;

        virtual float getYaw()
        {
            return direction.x == 0
                       ? (direction.z >= 0 ? 90.0f : 270.0f)
                       : glm::atan(direction.z / direction.x);
        }

        virtual float getPitch()
        {
            return glm::asin(direction.y);
        }

        virtual glm::vec3 getPosition()
        {
            return glm::vec3(position);
        }

        virtual glm::vec3 getDirection()
        {
            return glm::vec3(direction);
        }

        virtual void setYaw(float yaw)
        {
            setDirection(_3Dutils::directionOf(yaw, getPitch()));
        }

        virtual void setPitch(float pitch)
        {
            setDirection(_3Dutils::directionOf(getYaw(), pitch));
        }

        virtual void setYawPitch(float yaw, float pitch)
        {
            setDirection(_3Dutils::directionOf(yaw, pitch));
        }

        virtual void setDirection(glm::vec3 direction)
        {
            this->direction = direction;
        }

        virtual void setPosition(glm::vec3 position)
        {
            this->position = position;
        }

        void setWorld(kcomp::World *world)
        {
            this->world = world;
        }

        kcomp::World *getWorld() const
        {
            return world;
        }
    };
}
