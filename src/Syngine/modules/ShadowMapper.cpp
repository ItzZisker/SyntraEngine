#include "ShadowMapper.hpp"
#include "Skybox.hpp"
#include "Presets.hpp"

using namespace syng;

ShadowMapper::ShadowMapper(Shader& depthShader, GLuint width, GLuint height, glm::mat4 lightProj, glm::mat4 lightView) :
        shadowWidth(width), shadowHeight(height), lightProjection(lightProj), lightView(lightView), depthShader(depthShader) {}

ShadowMapper::ShadowMapper(Shader& depthShader, GLuint width, GLuint height) :
        ShadowMapper(depthShader, width, height, {}, {}) {
    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));
    glm::vec3 lightPos = -lightDir * 10.0f;
    glm::vec3 target = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float near_plane = 0.1f, far_plane = 75.0f;
    this->lightView = glm::lookAt(lightPos, target, up);
    this->lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane); 
}

ShadowMapper::ShadowMapper(Shader& depthShader, GLuint uv) : ShadowMapper(depthShader, uv, uv) {}

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
    
    PresetsTexel::TextureFilter(GL_TEXTURE_2D, GL_NEAREST);
    PresetsTexel::TextureParamST(GL_TEXTURE_2D, GL_CLAMP_TO_BORDER);

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

void ShadowMapper::pushUniforms(Shader& batchShader) {
    batchShader.use();
    batchShader.setMatrix4("lightSpaceMatrix", getLightSpaceMatrix(), 1, GL_FALSE);
    batchShader.setFloat("shadowStrength", strength);
    batchShader.setFloat("shadowBiasMin", biasMin);
    batchShader.setFloat("shadowBiasMax", biasMax);
    batchShader.setFloat("shadowPCFScale", pcfScale);
    batchShader.setInt("shadowPCFRadius", pcfRadius);
    batchShader.setTexture("shadowMap", GL_TEXTURE_2D, 7, getDepthMapTCB());
}

GLuint ShadowMapper::getDepthMapFBO() {
    return this->depthMapFBO;
}

GLuint ShadowMapper::getDepthMapTCB() {
    return this->depthMapTCB;
}

RenderTable<ShaderRenderable>* ShadowMapper::getDepthRenderTable() {
    return this->depthRendertable;
}