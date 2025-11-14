#include "CascadedShadowMapper.hpp"

#include "Presets.hpp"

#include "Syngine/modules/Scene.hpp"
#include "Syngine/ports/GLPort.h"

#include <vector>

using namespace syng;

CascadedShadowMapper::CascadedShadowMapper(Scene* scene, Shader& depthCSMShader, GLuint uv, glm::vec3 lightDir)
    : scene(scene), depthCSMShader(depthCSMShader), shadowWidth(uv), shadowHeight(uv), lightDir(lightDir) {
    float cameraFar = scene->getZFar();
    cascadeLevels[0] = cameraFar / 10.0f;
    cascadeLevels[1] = cameraFar / 2.0f;
}

CascadedShadowMapper::CascadedShadowMapper(Scene* scene, Shader& depthCSMShader, GLuint width, GLuint height, glm::vec3 lightDir)
    : scene(scene), depthCSMShader(depthCSMShader), shadowWidth(width), shadowHeight(height), lightDir(lightDir) {
    float cameraFar = scene->getZFar();
    cascadeLevels[0] = cameraFar / 10.0f;
    cascadeLevels[1] = cameraFar / 2.0f;
}

CascadedShadowMapper::~CascadedShadowMapper() {
    glDeleteFramebuffers(1, &depthMapsFBO);
    glDeleteTextures(1, &depthMapsTCB);
    delete depthRendertable;
}

void CascadedShadowMapper::create() {
    depthCSMShader.init();
    glGenFramebuffers(1, &depthMapsFBO);

    glGenTextures(1, &depthMapsTCB);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapsTCB);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, shadowWidth, shadowHeight, SG_CSM_CASCADE_COUNTS + 1,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapsFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapsTCB, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

void CascadedShadowMapper::renderDepth(Screenbuffer screen) {
    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapsFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    Screenbuffer shadowScreen(depthMapsFBO, shadowWidth, shadowHeight);
    auto renderFunc = [&](const std::string& key, ShaderRenderable* renderable){
        DepthRenderable *depthInstance = dynamic_cast<DepthRenderable*>(renderable);
        if (depthInstance) depthInstance->renderDepth(depthCSMShader, shadowScreen);
    };
    glCullFace(GL_FRONT);
    
    depthCSMShader.use();
    auto matrices = getLightSpaceMatrices();
    for (int i = 0; i < SG_CSM_CASCADE_COUNTS + 1; i++) {
        depthCSMShader.setMatrix4("lightSpaceMatrices[" + std::to_string(i) + "]", matrices[i], 1, GL_FALSE);
    }

    scene->getBatchRenderTable()->forEach(renderFunc);
    depthRendertable->forEach(renderFunc);

    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    glViewport(0, 0, screen.getWidth(), screen.getHeight());
}

std::vector<glm::vec4> CascadedShadowMapper::getFrustumCornersWorldSpace(const glm::mat4& projview) {
    const auto inv = glm::inverse(projview);
    std::vector<glm::vec4> frustumCorners;

    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }
    return frustumCorners;
}

std::vector<glm::vec4> CascadedShadowMapper::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view) {
    return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 CascadedShadowMapper::getLightSpaceMatrix(const float nearPlane, const float farPlane) {
    Scene_T snapshot = scene->getSnapshot();

    const auto proj = glm::perspective(glm::radians(snapshot.FOV), snapshot.aspectRatio, nearPlane,farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, scene->getCamera()->getViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners) center += glm::vec3(v);
    center /= corners.size();

    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners) {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }
    
    if (minZ < 0) minZ *= zMult;
    else minZ /= zMult;

    if (maxZ < 0) maxZ /= zMult;
    else  maxZ *= zMult;

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}

std::vector<glm::mat4> CascadedShadowMapper::getLightSpaceMatrices() {
    std::vector<glm::mat4> ret(SG_CSM_CASCADE_COUNTS + 1);
    for (size_t i = 0; i < SG_CSM_CASCADE_COUNTS + 1; ++i) {
        if (i == 0) {
            ret[i] = getLightSpaceMatrix(scene->getZNear(), cascadeLevels[i]);
        } else if (i < SG_CSM_CASCADE_COUNTS) {
            ret[i] = getLightSpaceMatrix(cascadeLevels[i - 1], cascadeLevels[i]);
        } else {
            ret[i] = getLightSpaceMatrix(cascadeLevels[i - 1], scene->getZFar());
        }
    }
    return ret;
}

void CascadedShadowMapper::pushUniforms(Shader& batchShader) {
    batchShader.use();
    for (int i = 0; i < SG_CSM_CASCADE_COUNTS; i++) {
        batchShader.setFloat("shadowCascadePlaneDistances[" + std::to_string(i) + "]", cascadeLevels[i]);
    }
    auto matrices = getLightSpaceMatrices();
    for (int i = 0; i < SG_CSM_CASCADE_COUNTS + 1; i++) {
        batchShader.setMatrix4("lightSpaceMatrices[" + std::to_string(i) + "]", matrices[i], 1, GL_FALSE);
    }
    batchShader.setFloat("shadowCascadeBiasModifier", cascadeBiasModifier);
    batchShader.setFloat("shadowStrength", strength);
    batchShader.setFloat("shadowBiasMin", biasMin);
    batchShader.setFloat("shadowBiasMax", biasMax);
    batchShader.setFloat("shadowPCFScale", pcfScale);
    batchShader.setFloat("shadowFarPlane", scene->getZFar());
    batchShader.setTexture("shadowMap", GL_TEXTURE_2D_ARRAY, 12, getDepthMapsTCB());
}

GLuint CascadedShadowMapper::getDepthMapsFBO() {
    return this->depthMapsFBO;
}

GLuint CascadedShadowMapper::getDepthMapsTCB() {
    return this->depthMapsTCB;
}

RenderTable<ShaderRenderable>* CascadedShadowMapper::getDepthRenderTable() {
    return this->depthRendertable;
}