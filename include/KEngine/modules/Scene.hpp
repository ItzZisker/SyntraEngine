#pragma once

#include "KEngine/KEngine.hpp"
#include "KEngine/engine/RenderTable.hpp"
#include "KEngine/modules/Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>

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

    void render(int FBO);

    void render(GameWindow* window, int FBO) override {
        render(FBO);
    }

    void render(Shader shader, int FBO) override {
        render(FBO);
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

    Camera* getCamera();

    glm::mat4 getProjection();

    glm::mat4 getViewMatrix();

    RenderTable<ShaderRenderable>* getBatchRenderTable();

    Shader getScreenShader();

    Shader getBatchShader();
};

struct WindowUserData {
    Scene* scene = nullptr;
    void* otherData = nullptr;
};