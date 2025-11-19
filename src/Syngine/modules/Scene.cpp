#include "Scene.hpp"

#include "CascadedShadowMapper.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"

#include "Syngine/engine/Config.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/ports/GLPort.h"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <SDL3/SDL_video.h>
#include <glm/ext/matrix_clip_space.hpp>

#include <string>

using namespace syng;

Scene::Scene(Camera* camera, Shader& batchShader, Shader& screenShader, Scene_T props)
    : camera(camera), batchShader(batchShader), screenShader(screenShader), snapshot(props) {
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, snapshot.width, snapshot.height);
    setPBR_NonIBLRadianceGGXSpecularLightToDirLight();
    setPBR_NonIBLRadianceLambertianIrradianceToDirLight();
    updateProjection();
    updateUniforms();
}

Scene::Scene(Camera* camera, Shader& batchShader, Shader& screenShader)
    : Scene(camera, batchShader, screenShader, {}) {}

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

void Scene::setPointLight(GLuint num, PointLight pointLight) {
    this->pointLights[num] = pointLight;
    updateUniforms();
}

void Scene::setSpotLight(GLuint num, SpotLight spotLight) {
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
        shadowMapper->renderDepth(screen);
        shadowMapper->pushUniforms(batchShader);
    }
    batchShader.use();
    batchShader.setMatrix4("view", camera->getViewMatrix(), 1, GL_FALSE);
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", camera->getPosition());

    if (brdflutTCB) batchShader.setTexture("texture_brdflut", GL_TEXTURE_2D, 13, brdflutTCB);
    if (hasIBL && envirrTCB) batchShader.setTexture("texture_irradance", GL_TEXTURE_CUBE_MAP, 14, envirrTCB);
    if (hasIBL && envTCB) batchShader.setTexture("texture_envmap", GL_TEXTURE_CUBE_MAP, 15, envTCB);
    batchShader.setBool("hasIBL", hasIBL);

    batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
        GameUtils::renderDV(renderable, this, batchShader, screen);
    });
}

void Scene::withShadows(CascadedShadowMapper* shadowMapper) {
    if (!shadowMapper || !shadowMapper->isCreated()) return;
    this->shadowMapper = shadowMapper;
    reloadShaders();
}

void Scene::onEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        int width, height;
        SDL_Window* current = SDL_GetWindowFromID(event.window.windowID);
        SDL_GetWindowSize(current, &width, &height);
        if (width != 0 && height != 0) {
            setScreenLayout(width, height); 
        }
    }
}

void Scene::updateProjection() {
    updateProjection(glm::perspective(
        glm::radians(snapshot.FOV),
        snapshot.aspectRatio,
        snapshot.zNear,
        snapshot.zFar
    ));
}

void Scene::updateUniforms() {
    screenShader.use();
    screenShader.setVec2f("screenSize", snapshot.width, snapshot.height);

    screenShader.setBool("hdrEnabled", false); // Default HDR values
    screenShader.setFloat("hdrExposure", 3.25f);
    screenShader.setBool("fxaaEnabled", false); // Default FXAA4 values
    screenShader.setFloat("fxaaReduceMin", 1.0f / 128.0f);
    screenShader.setFloat("fxaaReduceMul", 1.0f / 8.0f);
    screenShader.setFloat("fxaaSpanMax", 8.0f);

    batchShader.use();
    batchShader.setFloat("gamma", gamma);

    batchShader.setBool("hasNormalMap", false);
    batchShader.setFloat("roughnessConstrant", 4.0f); // Default roughness/parallax constrant
    batchShader.setBool("roughness", false);
    batchShader.setBool("parallax", false);

    batchShader.setFloat("shadowStrength", 0.5f); // Default shadow values
    batchShader.setFloat("shadowBiasMax", 0.05f);
    batchShader.setFloat("shadowBiasMin", 0.005f);
    batchShader.setFloat("shadowPCFScale", 1.0f);

    batchShader.setFloat("opacity", 1.0f); // Default material misc values
    batchShader.setFloat("specularStrength", 1.0f);
    batchShader.setFloat("shininess", 32.0f);
    batchShader.setFloat("parallaxMinLayers", 8.0f);
    batchShader.setFloat("parallaxMaxLayers", 32.0f);
    batchShader.setFloat("height_scale", 0.05f);

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

    batchShader.setBool("hasIBL", hasIBL);
    batchShader.setVec3f("NonIBLRadianceGGXSpecularLight", PBR_NonIBLRadianceGGXSpecularLight);
    batchShader.setVec3f("NonIBLRadianceLambertianIrradiance", PBR_NonIBLRadianceLambertianIrradiance);
    batchShader.setFloat("NonIBLRadianceGGXFactor", PBR_NonIBLRadianceGGXFactor);
    batchShader.setFloat("NonIBLRadianceLambertianFactor", PBR_NonIBLRadianceLambertianFactor);
}

void Scene::setIBL(bool hasIBL) {
    this->hasIBL = hasIBL;
    batchShader.use();
    batchShader.setBool("hasIBL", hasIBL);
}

void Scene::setPBR_NonIBLRadianceLambertianIrradianceToDirLight() {
    this->PBR_NonIBLRadianceLambertianIrradiance = dirLight.diffuse;
    batchShader.use();
    batchShader.setVec3f("NonIBLRadianceLambertianIrradiance", dirLight.diffuse);
}

void Scene::setPBR_NonIBLRadianceLambertianIrradiance(glm::vec3 nonIBLRadianceLambertianIrradiance) {
    this->PBR_NonIBLRadianceLambertianIrradiance = nonIBLRadianceLambertianIrradiance;
    batchShader.use();
    batchShader.setVec3f("NonIBLRadianceLambertianIrradiance", nonIBLRadianceLambertianIrradiance);
}

void Scene::setPBR_NonIBLRadianceLambertianFactor(float nonIBLRadianceLambertianFactor) {
    this->PBR_NonIBLRadianceLambertianFactor = nonIBLRadianceLambertianFactor;
    batchShader.use();
    batchShader.setFloat("NonIBLRadianceLambertianFactor", nonIBLRadianceLambertianFactor);
}

void Scene::setPBR_NonIBLRadianceGGXSpecularLightToDirLight() {
    this->PBR_NonIBLRadianceGGXSpecularLight = dirLight.specular;
    batchShader.use();
    batchShader.setVec3f("NonIBLRadianceGGXSpecularLight", dirLight.specular);
}

void Scene::setPBR_NonIBLRadianceGGXSpecularLight(glm::vec3 nonIBLRadianceGGXSpecularLight) {
    this->PBR_NonIBLRadianceGGXSpecularLight = nonIBLRadianceGGXSpecularLight;
    batchShader.use();
    batchShader.setVec3f("NonIBLRadianceGGXSpecularLight", nonIBLRadianceGGXSpecularLight);
}

void Scene::setPBR_NonIBLRadianceGGXFactor(float nonIBLRadianceGGXFactor) {
    this->PBR_NonIBLRadianceGGXFactor = nonIBLRadianceGGXFactor;
    batchShader.use();
    batchShader.setFloat("PBR_NonIBLRadianceGGXFactor", nonIBLRadianceGGXFactor);
}

void Scene::setGamma(float gamma) {
    this->gamma = gamma;
    batchShader.use();
    batchShader.setFloat("gamma", gamma);
}

void Scene::setEnvironmentTCB(GLuint TCB) {
    this->envTCB = TCB;
}

void Scene::setEnvironmentIrradianceTCB(GLuint TCB) {
    this->envirrTCB = TCB;
}

void Scene::setBRDFLUT_TCB(GLuint TCB) {
    this->brdflutTCB = TCB;
}

void Scene::updateProjection(glm::mat4 customPerspective) {
    projection = customPerspective;
    batchShader.use();
    batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
}

void Scene::setScreenLayout(int width, int height) {
    glViewport(0, 0, width, height);
    snapshot.width = width;
    snapshot.height = height;
    snapshot.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    screenShader.use();
    screenShader.setVec2f("screenSize", width, height);
    updateProjection();
}

void Scene::setZBufferLayout(float near, float far) {
    this->snapshot.zNear = near;
    this->snapshot.zFar = far;
    updateProjection();
}

void Scene::setAspectRatio(float aspectRatio) {
    this->snapshot.aspectRatio = aspectRatio;
    updateProjection();
}

void Scene::setFieldOfView(float FOVDegrees) {
    snapshot.FOV = FOVDegrees;
    updateProjection();
}

float Scene::getScreenWidth() {
    return snapshot.width;
}

float Scene::getScreenHeight() {
    return snapshot.height;
}

float Scene::getZNear() {
    return snapshot.zNear;
}

float Scene::getZFar() {
    return snapshot.zFar;
}

float Scene::getFieldOfViewDegrees() {
    return snapshot.FOV;
}

float Scene::getAspectRatio() {
    return snapshot.aspectRatio;
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
    Scene_T res = this->snapshot;
    res.cameraCoords = Coordination(camera->getTransform());
    return res;
}

CascadedShadowMapper* Scene::getShadowMapper() {
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

Shader& Scene::getBatchShader() {
    return batchShader;
}

Shader& Scene::getScreenShader() {
    return screenShader;
}