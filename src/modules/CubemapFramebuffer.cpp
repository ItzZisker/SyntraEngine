#include "Syngine/modules/CubemapFramebuffer.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

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

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                    sceneSize, sceneSize,
                    0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
    refractionShader.init();

    this->renderToParent = renderToParent;
    glGenTextures(1, &cubemapTexture);

    for (unsigned int i = 0; i < 6; i++) {
        createFramebuffer(i);
    }
    for (auto& func : initTasks) {
        func(this);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    addRenderTask([&](unsigned int FBO, const Coordination& sideView) {
        Shader batchShader = scene->getBatchShader();

        glm::vec3 cameraPos = scene->getCamera()->getPosition();
        glm::vec3 cameraDir = scene->getCamera()->getDirection();
        glm::mat4 projection = glm::perspective(fieldOfView, aspectRatio, zNear, zFar);
        glm::mat4 view = glm::lookAt(sideView.getPosition(), sideView.getPosition() + sideView.getDirection(), sideView.getUp());

        batchShader.use();
        batchShader.setMatrix4("view", view, 1, GL_FALSE);
        batchShader.setMatrix4("projection", projection, 1, GL_FALSE);
        batchShader.setVec3f("cameraPos", cameraPos);
        batchShader.setVec3f("spotLight.position", cameraPos);
        batchShader.setVec3f("spotLight.direction", cameraDir);

        scene->getBatchRenderTable()->forEach([&](const std::string& key, ShaderRenderable* renderable) {
            GameUtils::renderDV(renderable, getSnapshot(sideView), batchShader, FBO);
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

void CubemapFramebuffer::render(int parentFBO) {
    reflectionRendertable->forEach([&](const std::string& key, ShaderRenderable* renderable){
        if (renderToParent && GameUtils::shouldDiscard(renderable, scene)) {
            return;
        }
        renderCubemap(renderable, parentFBO);

        if (renderToParent) {            
            glViewport(0, 0, scene->getScreenWidth(), scene->getScreenHeight());
            scene->getCamera()->updateViewMatrix();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

            reflectionShader.use();
            reflectionShader.setMatrix4("view", scene->getCamera()->getViewMatrix(), 1, GL_FALSE);
            reflectionShader.setMatrix4("projection", scene->getProjection(), 1, GL_FALSE);
            reflectionShader.setVec3f("cameraPos", scene->getCamera()->getPosition());
            reflectionShader.setInt("environmentMap", 0);

            GameUtils::renderDV(renderable, scene, reflectionShader, parentFBO);
        }
    });
    refractionRendertable->forEach([&](const std::string& key, ShaderRenderable* renderable){
        if (renderToParent && GameUtils::shouldDiscard(renderable, scene)) {
            return;
        }
        renderCubemap(renderable, parentFBO);

        if (renderToParent) {            
            glViewport(0, 0, scene->getScreenWidth(), scene->getScreenHeight());
            scene->getCamera()->updateViewMatrix();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

            refractionShader.use();
            refractionShader.setMatrix4("view", scene->getCamera()->getViewMatrix(), 1, GL_FALSE);
            refractionShader.setMatrix4("projection", scene->getProjection(), 1, GL_FALSE);
            refractionShader.setVec3f("cameraPos", scene->getCamera()->getPosition());
            refractionShader.setInt("environmentMap", 0);
            refractionShader.setFloat("ior", 1.5f);

            GameUtils::renderDV(renderable, scene, refractionShader, parentFBO);
        }
    });
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