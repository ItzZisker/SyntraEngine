#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include <Syngine/modules/Framebuffer.hpp>
#include <iostream>

Framebuffer::Framebuffer(Scene* scene) : scene(scene) {}

Framebuffer::~Framebuffer() {
    initTasks.clear();
    renderTasks.clear();
    delete renderTable;
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteTextures(1, &TCB);

    if (outputToQuad) {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
    }
}

void Framebuffer::create(GameWindow* window, bool outputToScreenShader) {
    this->outputToQuad = outputToScreenShader;

    if (FBO || RBO || TCB) {
        std::cerr << "ERROR::FRAMEBUFFER::Already Created" << std::endl;
        return;
    }

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window->getWindowWidth(), window->getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TCB, 0);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window->getWindowWidth(), window->getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    for (auto& func : initTasks) {
        func(this);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (outputToQuad) {
        float quadVertices[] = {
            // positions // texCoords
        -1.0f,   1.0f,  0.0f,  1.0f,
        -1.0f,  -1.0f,  0.0f,  0.0f,
        1.0f,  -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 1.0f, 1.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    
    RenderTable<ShaderRenderable>* renderTableCopy = renderTable;
    Scene *scene = this->scene;
    addRenderTask([scene, renderTableCopy](Framebuffer* framebuffer){
        renderTableCopy->forEach([&](const std::string& key, ShaderRenderable* renderable){
            GameUtils::renderDV(renderable, scene, scene->getBatchShader(), framebuffer->getFBO());
        });
    });
}

void Framebuffer::addInitTask(std::function<void(Framebuffer *)> task) {
    initTasks.push_back(task);
}

void Framebuffer::addRenderTask(std::function<void(Framebuffer *)> task) {
    renderTasks.push_back(task);
}

void Framebuffer::render(int parentFBO) {
    if (!FBO || !RBO) return;

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (auto& func : renderTasks) {
        func(this);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, parentFBO);

    if (outputToQuad) {
        glViewport(0, 0, scene->getScreenWidth(), scene->getScreenHeight());
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        scene->getScreenShader().use();
        scene->getScreenShader().setInt("screenTexture", 0);

        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, TCB);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

RenderTable<ShaderRenderable>* Framebuffer::getRenderTable() {
    return this->renderTable;
}

unsigned int Framebuffer::getRBO() {
    return !this->RBO ? 0 : this->RBO;
}

unsigned int Framebuffer::getFBO() {
    return !this->FBO ? 0 : this->FBO;
}