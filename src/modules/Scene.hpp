#pragma once

#include "EventHandler.hpp"
#include "Screenbuffer.hpp"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>

namespace syng
{
class ShadowMapper;

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

struct DirLight {
    glm::vec3 direction = {-0.5f, -1.0f, -0.5f};
    glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse = {0.4f, 0.4f, 0.4f};
    glm::vec3 specular = {0.5f, 0.5f, 0.5f};
};

class Scene : public SDL_EventHandler, public DuplexRenderable
{
private:
    ShadowMapper* shadowMapper = nullptr;

    SDL_Window* callbackSDLWindow = nullptr;

    Shader screenShader = Shader("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    Shader batchShader = Shader("shaders/batchVertex.glsl", "shaders/batchFrag.glsl");

    RenderTable<ShaderRenderable>* batchRenderTable = new RenderTable<ShaderRenderable>;
    Camera* camera;
    DirLight dirLight;

    int screenWidth;
    int screenHeight;
    float near;
    float far;
    float fieldOfView;
    float aspectRatio;

    glm::mat4 projection;
    
    void setupShaders();

    void updateProjection();
public:
    Scene(Camera* camera, GameWindow* window);

    Scene(Camera* camera, float FOVDegrees, float near, float far, int width, int height);

    void render(Screenbuffer screen);

    void render(GameWindow* window) override {
        render(*window);
    }

    void render(Shader shader, Screenbuffer screen = {}) override {
        render(screen);
    }

    void onEvent(const SDL_Event& event) override;

    void withShadows(ShadowMapper* shadowMapper);

    void updateProjection(glm::mat4 customPerspective);

    void updateLights();

    void setDirectionalLight(DirLight light);

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

    ShadowMapper* getShadowMapper();

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
}