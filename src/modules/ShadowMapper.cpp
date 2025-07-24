#include "modules/ShadowMapper.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "modules/Skybox.hpp"
#include "utils/GameUtils.hpp"

using namespace syng;

ShadowMapper::ShadowMapper(unsigned int width, unsigned int height, glm::mat4 lightProj, glm::mat4 lightView) :
                            shadowWidth(width), shadowHeight(height), lightProjection(lightProj), lightView(lightView) {}

ShadowMapper::ShadowMapper(unsigned int width, unsigned int height) : ShadowMapper(width, height, {}, {}) {
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    glm::vec3 lightPos = -lightDir * 10.0f;
    glm::vec3 target = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float near_plane = 0.1f, far_plane = 75.0f;
    this->lightView = glm::lookAt(lightPos, target, up);
    this->lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane); 
}

ShadowMapper::ShadowMapper(unsigned int uv) : ShadowMapper(uv, uv) {}

ShadowMapper::~ShadowMapper() {
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMapTCB);
}

void ShadowMapper::create() {
    depthShader.init();
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMapTCB);
    glBindTexture(GL_TEXTURE_2D, depthMapTCB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTCB, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

const glm::mat4 ShadowMapper::getLightSpaceMatrix() {
    return lightProjection * lightView;
}

void ShadowMapper::renderDepth(Screenbuffer screen, Scene *scene) {
    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    Screenbuffer shadowScreen(depthMapFBO, shadowWidth, shadowHeight);
    auto renderFunc = [&](const std::string& key, ShaderRenderable* renderable){
        if (!dynamic_cast<Skybox*>(renderable)) {
            renderable->render(depthShader, shadowScreen);
        }
    };
    glCullFace(GL_FRONT);
    
    depthShader.use();
    depthShader.setMatrix4("lightSpaceMatrix", getLightSpaceMatrix(), 1, GL_FALSE);

    scene->getBatchRenderTable()->forEach(renderFunc);
    depthRendertable->forEach(renderFunc);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    glViewport(0, 0, screen.getWidth(), screen.getHeight());
}

unsigned int ShadowMapper::getDepthMapFBO() {
    return this->depthMapFBO;
}

unsigned int ShadowMapper::getDepthMapTCB() {
    return this->depthMapTCB;
}

RenderTable<ShaderRenderable>* ShadowMapper::getDepthRenderTable() {
    return this->depthRendertable;
}