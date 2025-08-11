#pragma once

#include "EventHandler.hpp"
#include "Screenbuffer.hpp"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <vector>

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

    void boost(float scl) {
        ambient *= scl;
        diffuse *= scl;
        specular *= scl;
    }
};

struct PointLight {
    glm::vec3 position = {0.0f, 1.0f, 0.0f};
    glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void boost(float scl) {
        ambient *= scl;
        diffuse *= scl;
        specular *= scl;
    }
};

struct SpotLight {
    glm::vec3 position = {0.0f, 1.0f, 0.0f};
    glm::vec3 direction = {-0.5f, -1.0f, -0.5f};
    glm::vec3 ambient = {0.05f, 0.05f, 0.05f};
    glm::vec3 diffuse = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {1.0f, 1.0f, 1.0f};
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(15.0f));

    void boost(float scl) {
        ambient *= scl;
        diffuse *= scl;
        specular *= scl;
    }
};

class Scene : public SDL_EventHandler, public DuplexRenderable
{
private:
    ShadowMapper* shadowMapper = nullptr;

    Shader screenShader = Shader("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    Shader batchShader = Shader("shaders/batchVertex.glsl", "shaders/batchFrag.glsl");

    RenderTable<ShaderRenderable>* batchRenderTable = new RenderTable<ShaderRenderable>;
    Camera* camera;

    DirLight dirLight;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    
    int screenWidth;
    int screenHeight;
    float near;
    float far;
    float fieldOfView;
    float aspectRatio;
    float gamma = 1.1f;

    glm::mat4 projection;

    void updateProjection();
public:
    Scene(Camera* camera, GameWindow* window);

    Scene(Camera* camera, float FOVDegrees, float near, float far, int width, int height);
    
    void reloadShaders();

    void setupShaders() {
        reloadShaders();
    }

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
    void updateUniforms();
    void setGamma(float gamma);

    void setDirectionalLight(DirLight light);
    void setPointLights(std::vector<PointLight> pointLights);
    void setSpotLights(std::vector<SpotLight> spotLights);
    void setPointLight(unsigned int index, PointLight light);
    void setSpotLight(unsigned int index, SpotLight light);

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

    std::vector<PointLight> getPointLights();
    std::vector<SpotLight> getSpotLights();

    DirLight getDirectionalLight();
    PointLight getPointLight(unsigned int index);
    SpotLight getSpotLight(unsigned int index);

    ShadowMapper* getShadowMapper();

    Camera* getCamera();
    Scene_T getSnapshot();

    glm::mat4 getProjection();
    glm::mat4 getViewMatrix();

    Shader getScreenShader();
    Shader getBatchShader();

    RenderTable<ShaderRenderable>* getBatchRenderTable();
};
}