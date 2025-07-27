#include "modules/Scene.hpp"
#include "SDL3/SDL_video.h"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/ShadowMapper.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include "utils/GameUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "engine/Config.hpp"

using namespace syng;

Scene::Scene(Camera* camera, GameWindow* window)
    : camera(camera),
      screenWidth(window->getWidth()),
      screenHeight(window->getHeight()),
      near(0.1f),
      far(100.0f),
      fieldOfView(65.0f) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, screenWidth, screenHeight);
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    updateProjection();
    setupShaders();
}

Scene::Scene(Camera* camera, float FOVDegrees, float near, float far, int width, int height)
    : camera(camera),
      screenWidth(width),
      screenHeight(height),
      near(near),
      far(far),
      fieldOfView(FOVDegrees) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, screenWidth, screenHeight);
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    updateProjection();
    setupShaders();
}

void Scene::setDirectionalLight(DirLight light) {
    this->dirLight = light;
    updateLights();
}

void Scene::setupShaders() {
    screenShader.init();
    screenShader.use();
    screenShader.setVec2f("uv", screenWidth, screenHeight);

    batchShader.init({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, "1"},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, "0"},
        {SHADER_BATCH_KEY_HAS_SHADOWS, "0"}
    });
    setDirectionalLight({});
    batchShader.setVec3f("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("pointLights[0].diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("pointLights[0].constant", 1.0f);
    batchShader.setFloat("pointLights[0].linear", 0.09f);
    batchShader.setFloat("pointLights[0].quadratic", 0.032f);

    batchShader.setVec3f("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("spotLights[0].diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("spotLights[0].constant", 1.0f);
    batchShader.setFloat("spotLights[0].linear", 0.09f);
    batchShader.setFloat("spotLights[0].quadratic", 0.032f);
    batchShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    batchShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));

    batchShader.setVec3f("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("spotLights[1].constant", 1.0f);
    batchShader.setFloat("spotLights[1].linear", 0.09f);
    batchShader.setFloat("spotLights[1].quadratic", 0.032f);
    batchShader.setFloat("spotLights[1].cutOff", glm::cos(glm::radians(12.5f)));
    batchShader.setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(15.0f)));
}

void Scene::render(Screenbuffer screen) {
    camera->updateViewMatrix();

    if (shadowMapper && shadowMapper->isCreated()) {
        shadowMapper->renderDepth(screen, this);
        batchShader.use();
        batchShader.setMatrix4("lightSpaceMatrix", shadowMapper->getLightSpaceMatrix(), 1, GL_FALSE);
        batchShader.setFloat("shadowBiasMin", shadowMapper->biasMin);
        batchShader.setFloat("shadowBiasMax", shadowMapper->biasMax);
        batchShader.setFloat("shadowBias", shadowMapper->biasMax);
        batchShader.setTexture("shadowMap", GL_TEXTURE_2D, 2, shadowMapper->getDepthMapTCB());
    }
    batchShader.use();
    batchShader.setMatrix4("view", camera->getViewMatrix(), 1, GL_FALSE);
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", camera->getPosition());
    batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
        GameUtils::renderDV(renderable, this, batchShader, screen);
    });
}

void Scene::withShadows(ShadowMapper* shadowMapper) {
    if (!shadowMapper->isCreated()) return;
    this->shadowMapper = shadowMapper;
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, "0"},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, "0"},
        {SHADER_BATCH_KEY_HAS_SHADOWS, SHADER_VAL_ON}
    });
    updateLights();
}

void Scene::onEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        int width, height;
        SDL_Window* current = SDL_GetWindowFromID(event.window.windowID);
        SDL_GetWindowSize(current, &width, &height);
        if (width != 0 && height != 0)
            setScreenLayout(width, height);
    }
}

void Scene::updateProjection() {
    updateProjection(glm::perspective(glm::radians(fieldOfView), aspectRatio, near, far));
}

void Scene::updateLights() {
    batchShader.use();
    batchShader.setVec3f("dirLight.direction", dirLight.direction);
    batchShader.setVec3f("dirLight.ambient", dirLight.ambient);
    batchShader.setVec3f("dirLight.diffuse", dirLight.diffuse);
    batchShader.setVec3f("dirLight.specular", dirLight.specular);
}

void Scene::updateProjection(glm::mat4 customPerspective) {
    projection = customPerspective;
    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
}

void Scene::setScreenLayout(int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
    aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    screenShader.use();
    screenShader.setVec2f("resolution", width, height);
    updateProjection();
}

void Scene::setZBufferLayout(float near, float far) {
    this->near = near;
    this->far = far;
    updateProjection();
}

void Scene::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    updateProjection();
}

void Scene::setFieldOfView(float FOVDegrees) {
    fieldOfView = FOVDegrees;
    updateProjection();
}

float Scene::getScreenWidth() {
    return screenWidth;
}

float Scene::getScreenHeight() {
    return screenHeight;
}

float Scene::getZNear() {
    return near;
}

float Scene::getZFar() {
    return far;
}

float Scene::getFieldOfViewDegrees() {
    return fieldOfView;
}

float Scene::getAspectRatio() {
    return aspectRatio;
}

Scene_T Scene::getSnapshot() {
    Scene_T res;
    res.cameraPos = camera->getPosition();
    res.cameraDir = camera->getDirection();
    res.cameraUp = camera->getUp();
    res.cameraRight = camera->getRight();
    res.aspectRatio = aspectRatio;
    res.FOV = fieldOfView;
    res.zNear = near;
    res.zFar = far;
    return res;
}

ShadowMapper* Scene::getShadowMapper() {
    return shadowMapper;
}

Camera* Scene::getCamera() {
    return camera;
}

RenderTable<ShaderRenderable>* Scene::getBatchRenderTable() {
    return batchRenderTable;
}

glm::mat4 Scene::getProjection() {
    return projection;
}

glm::mat4 Scene::getViewMatrix() {
    return camera->getViewMatrix();
}

Shader Scene::getBatchShader() {
    return batchShader;
}

Shader Scene::getScreenShader() {
    return screenShader;
}