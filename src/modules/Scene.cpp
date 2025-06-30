#include "Syngine/modules/Scene.hpp"
#include "GLFW/glfw3.h"
#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <ostream>
#include "Syngine/engine/Config.hpp"

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

Scene::~Scene() {
    glfwSetWindowUserPointer(callbacksGLFWWindow, lastWindowUserData);
}

void Scene::setupShaders() {
    screenShader.init();
    screenShader.use();
    screenShader.setVec2f("resolution", screenWidth, screenHeight);

    batchShader.init({
        {SHADER_BATCH_KEY_NR_POINT_LIGHTS, "1"},
        {SHADER_BATCH_KEY_NR_SPOT_LIGHTS, "1"}
    });
    batchShader.use();
    batchShader.setFloat("shininess", 32.0f);
    batchShader.setVec3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
    batchShader.setVec3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    batchShader.setVec3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    batchShader.setVec3f("dirLight.specular", 0.5f, 0.5f, 0.5f);

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

    batchShader.use();
    batchShader.setMatrix4("view", camera->getViewMatrix(), 1, GL_FALSE);
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", camera->getPosition());
    batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
        GameUtils::renderDV(renderable, this, batchShader, screen);
    });
}

void Scene::installCallbacks(GameWindow* window) {
    GLFWwindow* glfwWindow = window->getGLFWWindowPtr();

    lastWindowUserData = glfwGetWindowUserPointer(glfwWindow);

    WindowUserData* data = new WindowUserData();
    data->scene = this;
    
    glfwSetWindowUserPointer(glfwWindow, data);
    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* glfwWindow, int width, int height) {
        WindowUserData* data = static_cast<WindowUserData*>(glfwGetWindowUserPointer(glfwWindow));
        if (data && data->scene) {
            data->scene->setScreenLayout(width, height);
        }
    });
}

void Scene::updateProjection() {
    updateProjection(glm::perspective(glm::radians(fieldOfView), aspectRatio, near, far));
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