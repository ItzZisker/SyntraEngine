#pragma once

#include "Screenbuffer.hpp"
#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>

struct Scene_T {
    glm::vec3 cameraPos;
    glm::vec3 cameraDir;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    float zNear;
    float zFar;
    float aspectRatio;
    float FOV;
};

class Scene : public DuplexRenderable
{
private:
    GLFWwindow* callbacksGLFWWindow = nullptr;
    void* lastWindowUserData = nullptr;

    Shader screenShader = Shader("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    Shader batchShader = Shader("shaders/batchVertex.glsl", "shaders/batchFrag.glsl");

    RenderTable<ShaderRenderable>* batchRenderTable = new RenderTable<ShaderRenderable>;
    Camera* camera;

    int screenWidth;
    int screenHeight;
    float near;
    float far;
    float fieldOfView;
    float aspectRatio;

    glm::mat4 projection;
    
    void setupShaders();

    void updateProjection();

    void glfw_framebuffer_resize_callback(GLFWwindow *glfwWindow, int width, int height);
public:
    Scene(Camera* camera, GameWindow* window);

    Scene(Camera* camera, float FOVDegrees, float near, float far, int width, int height);

    ~Scene();

    void render(Screenbuffer screen);

    void render(GameWindow* window) override {
        render(*window);
    }

    void render(Shader shader, Screenbuffer screen = {}) override {
        render(screen);
    }

    void installCallbacks(GameWindow* window);

    void updateProjection(glm::mat4 customPerspective);

    void setScreenLayout(int width, int height);

    void setZBufferLayout(float near, float far);

    void setAspectRatio(float aspectRatio);

    void setFieldOfView(float FOVDegrees);

    float getScreenWidth();

    float getScreenHeight();

    float getZNear();

    float getZFar();

    float getFieldOfViewDegrees();

    float getAspectRatio();

    Scene_T getSnapshot();

    Camera* getCamera();

    glm::mat4 getProjection();

    glm::mat4 getViewMatrix();

    RenderTable<ShaderRenderable>* getBatchRenderTable();

    Shader getScreenShader();

    Shader getBatchShader();
};

struct WindowUserData {
    Scene* scene = nullptr;
    void* suffix = nullptr;
};