#include "Syngine/modules/Scene.hpp"
#include "GLFW/glfw3.h"
#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene(Camera* camera, GameWindow* window)
    : camera(camera),
      screenWidth(window->getWindowWidth()),
      screenHeight(window->getWindowHeight()),
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

    batchShader.init();
    batchShader.use();
    batchShader.setFloat("shininess", 32.0f);
    batchShader.setVec3f("dirLight.direction", -0.2f, -1.0f, -0.3f);
    batchShader.setVec3f("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    batchShader.setVec3f("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    batchShader.setVec3f("dirLight.specular", 0.5f, 0.5f, 0.5f);
    batchShader.setVec3f("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    batchShader.setVec3f("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    batchShader.setVec3f("spotLight.specular", 1.0f, 1.0f, 1.0f);
    batchShader.setFloat("spotLight.constant", 1.0f);
    batchShader.setFloat("spotLight.linear", 0.09f);
    batchShader.setFloat("spotLight.quadratic", 0.032f);
    batchShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    batchShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

void Scene::render(int FBO) {
    camera->updateViewMatrix();
    glm::vec3 cameraPos = camera->getPosition();

    batchShader.use();
    batchShader.setMatrix4("view", camera->getViewMatrix(), 1, GL_FALSE);
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", cameraPos);
    batchShader.setVec3f("spotLight.position", cameraPos);
    batchShader.setVec3f("spotLight.direction", camera->getDirection());
    batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
        if (!GameUtils::shouldDiscard(renderable, this)) {
            renderable->render(batchShader, FBO);
        }
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