#include "modules/Scene.hpp"
#include "SDL3/SDL_video.h"
#include "Scene.hpp"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/ShadowMapper.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include "utils/GameUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
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
}

void Scene::setDirectionalLight(DirLight light) {
    this->dirLight = light;
    updateUniforms();
}

void Scene::setPointLights(std::vector<PointLight> pointLights) {
    this->pointLights = pointLights;
    updateUniforms();
}

void Scene::setSpotLights(std::vector<SpotLight> spotLights) {
    this->spotLights = spotLights;
    updateUniforms();
}

void Scene::setPointLight(unsigned int num, PointLight pointLight) {
    this->pointLights[num] = pointLight;
    updateUniforms();
}

void Scene::setSpotLight(unsigned int num, SpotLight spotLight) {
    this->spotLights[num] = spotLight;
    updateUniforms();
}

void Scene::reloadShaders() {
    screenShader.reloadProgram();
    batchShader.reloadProgram({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, std::to_string(pointLights.size())},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, std::to_string(spotLights.size())},
        {SHADER_BATCH_KEY_HAS_SHADOWS, shadowMapper && shadowMapper->isCreated() ? SHADER_VAL_ON : SHADER_VAL_OFF}
    });
    updateUniforms();
}

void Scene::render(Screenbuffer screen) {
    camera->updateViewMatrix();

    if (shadowMapper && shadowMapper->isCreated()) {
        shadowMapper->renderDepth(screen, this);
        batchShader.use();
        batchShader.setMatrix4("lightSpaceMatrix", shadowMapper->getLightSpaceMatrix(), 1, GL_FALSE);
        batchShader.setFloat("shadowStrength", shadowMapper->strength);
        batchShader.setFloat("shadowBiasMin", shadowMapper->biasMin);
        batchShader.setFloat("shadowBiasMax", shadowMapper->biasMax);
        batchShader.setTexture("shadowMap", GL_TEXTURE_2D, 7, shadowMapper->getDepthMapTCB());
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
    if (!shadowMapper || !shadowMapper->isCreated()) return;
    this->shadowMapper = shadowMapper;
    reloadShaders();
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

void Scene::updateUniforms() {
    screenShader.use();
    screenShader.setVec2f("screenSize", screenWidth, screenHeight);
    batchShader.use();
    batchShader.setVec3f("dirLight.direction", dirLight.direction);
    batchShader.setVec3f("dirLight.ambient", dirLight.ambient);
    batchShader.setVec3f("dirLight.diffuse", dirLight.diffuse);
    batchShader.setVec3f("dirLight.specular", dirLight.specular);
    for (unsigned int i = 0; i < pointLights.size(); i++) {
        PointLight pointLight = pointLights[i];
        std::string num = std::to_string(i);
        batchShader.setVec3f("pointLights[" + num + "].position", pointLight.position);
        batchShader.setVec3f("pointLights[" + num + "].ambient", pointLight.ambient);
        batchShader.setVec3f("pointLights[" + num + "].diffuse", pointLight.diffuse);
        batchShader.setVec3f("pointLights[" + num + "].specular", pointLight.specular);
        batchShader.setFloat("pointLights[" + num + "].constant", pointLight.constant);
        batchShader.setFloat("pointLights[" + num + "].linear", pointLight.linear);
        batchShader.setFloat("pointLights[" + num + "].quadratic", pointLight.quadratic);
    }
    for (unsigned int i = 0; i < spotLights.size(); i++) {
        SpotLight spotLight = spotLights[i];
        std::string num = std::to_string(i);
        batchShader.setVec3f("spotLights[" + num + "].position", spotLight.position);
        batchShader.setVec3f("spotLights[" + num + "].direction", spotLight.direction);
        batchShader.setVec3f("spotLights[" + num + "].ambient", spotLight.ambient);
        batchShader.setVec3f("spotLights[" + num + "].diffuse", spotLight.diffuse);
        batchShader.setVec3f("spotLights[" + num + "].specular", spotLight.specular);
        batchShader.setFloat("spotLights[" + num + "].constant", spotLight.constant);
        batchShader.setFloat("spotLights[" + num + "].linear", spotLight.linear);
        batchShader.setFloat("spotLights[" + num + "].quadratic", spotLight.quadratic);
        batchShader.setFloat("spotLights[" + num + "].cutOff", spotLight.cutOff);
    }
}

void Scene::setGamma(float gamma) {
    this->gamma = gamma;
    screenShader.use();
    screenShader.setFloat("gamma", gamma);
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
    screenShader.setVec2f("screenSize", width, height);
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

DirLight Scene::getDirectionalLight() {
    return dirLight;
}

std::vector<PointLight> Scene::getPointLights() {
    return pointLights;
}

std::vector<SpotLight> Scene::getSpotLights() {
    return spotLights;
}

PointLight Scene::getPointLight(unsigned int num) {
    return pointLights[num];
}

SpotLight Scene::getSpotLight(unsigned int num) {
    return spotLights[num];
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