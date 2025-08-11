#include "modules/CubemapFramebuffer.hpp"
#include "Presets.hpp"
#include "engine/Config.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include "utils/GameUtils.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

using namespace syng;

CubemapFramebuffer::CubemapFramebuffer(Scene* scene) : scene(scene) {}

CubemapFramebuffer::~CubemapFramebuffer() {
    glDeleteFramebuffers(6, FBO);
    glDeleteRenderbuffers(6, RBO);
    glDeleteTextures(1, &cubemapTexture);

    initTasks.clear();
    renderTasks.clear();

    delete reflectionRendertable;
    delete refractionRendertable;

    reflectionShader.disposeProgram();
    refractionShader.disposeProgram();
}

void CubemapFramebuffer::createFramebuffer(int index) {
    glGenFramebuffers(1, &FBO[index]);
    glGenRenderbuffers(1, &RBO[index]);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO[index]);

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    PresetsTexel::TextureFilter(GL_TEXTURE_CUBE_MAP, GL_LINEAR);
    PresetsTexel::TextureParamSTR(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, cubemapTexture, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, RBO[index]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sceneSize, sceneSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO[index]);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::CUBEMAP_FRAMEBUFFER::Framebuffer index="<< index<<" is not complete!" << std::endl;
    }
}

void CubemapFramebuffer::create(bool renderToParent) {
    reflectionShader.init();
    refractionShader.init({
        {SHADER_REFRAC_KEY_DYNAMIC_OPACITY, SHADER_VAL_OFF}
    });

    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                     sceneSize, sceneSize,
                     0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    for (unsigned int i = 0; i < 6; i++) {
        createFramebuffer(i);
    }
    for (auto& func : initTasks) {
        func(this);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    onCreate(sceneSize, sceneSize, renderToParent);
    addRenderTask([&](unsigned int FBO, const Coordination& sideView) {
        Shader batchShader = scene->getBatchShader();

        glm::mat4 projection = glm::perspective(fieldOfView, aspectRatio, zNear, zFar);
        glm::mat4 view = glm::lookAt(sideView.getPosition(), sideView.getPosition() + sideView.getDirection(), sideView.getUp());

        batchShader.use();
        batchShader.setMatrix4("view", view, 1, GL_FALSE);
        batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
        batchShader.setVec3f("cameraPos", sideView.getPosition());

        scene->getBatchRenderTable()->forEach([&](const std::string& key, ShaderRenderable* renderable) {
            GameUtils::renderDV(renderable, getSnapshot(sideView), batchShader, Screenbuffer(FBO));
        });
    });
}

void CubemapFramebuffer::renderCubemap(ShaderRenderable* renderable, int parentFBO) {
    glm::vec3 position = scene->getCamera()->getPosition();

    if (Coordination* coords = dynamic_cast<Coordination*>(renderable)) {
        position = coords->getPosition();
    }

    Coordination sideViews[6] = {
        {position, glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)},
        {position, glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)},
        {position, glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)},
        {position, glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)},
        {position, glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)},
        {position, glm::vec3(0, 0,-1), glm::vec3(0, -1, 0)}
    };

    for (unsigned int i = 0; i < SG_CUBEMAP_SIDES; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);
        glViewport(0, 0, sceneSize, sceneSize);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& task : renderTasks) {
            task(FBO[i], sideViews[i]);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, parentFBO);
}

void CubemapFramebuffer::render(Screenbuffer screen) {
    auto renderJob = [&](const std::string& key, ShaderRenderable* renderable, Shader shader){
        if (outputToParent && GameUtils::shouldDiscard(renderable, scene)) {
            return;
        }
        renderCubemap(renderable, screen.getFBO());

        if (outputToParent) {          
            glViewport(0, 0, screen.getWidth(), screen.getHeight());
            scene->getCamera()->updateViewMatrix();

            shader.use();
            shader.setMatrix4("view", scene->getCamera()->getViewMatrix(), 1, GL_FALSE);
            shader.setMatrix4("projection", scene->getProjection(), 1, GL_FALSE);
            shader.setVec3f("cameraPos", scene->getCamera()->getPosition());
            shader.setTexture("environmentMap", GL_TEXTURE_CUBE_MAP, 0, cubemapTexture);

            GameUtils::renderDV(renderable, scene, shader, screen.getFBO());
        }
    };
    reflectionRendertable->forEach([&](const std::string& key, ShaderRenderable* renderable){renderJob(key, renderable, reflectionShader);});
    refractionRendertable->forEach([&](const std::string& key, ShaderRenderable* renderable){renderJob(key, renderable, refractionShader);});
}

Scene_T CubemapFramebuffer::getSnapshot(Coordination cubemapSideView) {
    Scene_T res;
    res.cameraPos = cubemapSideView.getPosition();
    res.cameraDir = cubemapSideView.getDirection();
    res.cameraUp = cubemapSideView.getUp();
    res.cameraRight = cubemapSideView.getRight();
    res.aspectRatio = aspectRatio;
    res.FOV = fieldOfView;
    res.zNear = zNear;
    res.zFar = zFar;
    return res;
}

void CubemapFramebuffer::addInitTask(std::function<void(CubemapFramebuffer*)> task) {
    initTasks.push_back(task);
}

void CubemapFramebuffer::addRenderTask(std::function<void(unsigned int FBO, const Coordination& sideView)> task) {
    renderTasks.push_back(task);
}

RenderTable<ShaderRenderable>* CubemapFramebuffer::getReflectionRenderTable() {
    return this->reflectionRendertable;
}

RenderTable<ShaderRenderable>* CubemapFramebuffer::getRefractionRenderTable() {
    return this->refractionRendertable;
}

unsigned int CubemapFramebuffer::getCubemapTexture() const {
    return this->cubemapTexture;
}

const unsigned int* CubemapFramebuffer::getFBOs() const {
    return &this->FBO[0];
}

const unsigned int* CubemapFramebuffer::getRBOs() const {
    return &this->RBO[0];
}