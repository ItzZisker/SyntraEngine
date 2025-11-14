#include "EnvironmentMap.hpp"
#include "Shader.hpp"

#include "Syngine/modules/CascadedShadowMapper.hpp"
#include "Syngine/modules/Presets.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/ports/GLPort.h"
#include "Syngine/utils/GameUtils.hpp"

#include "glm/ext/matrix_clip_space.hpp"

EnvironmentMap::EnvironmentMap(Scene *scene, Shader& irradianceShader) : scene(scene), irradianceShader(irradianceShader) {}

EnvironmentMap::~EnvironmentMap() {
    glDeleteFramebuffers(1, &envFBO);
    glDeleteRenderbuffers(1, &envRBO);
    glDeleteTextures(1, &envTCB);
    glDeleteTextures(1, &irrTCB);
    delete this->cube;
}

void EnvironmentMap::create(int resolution, int irradianceResolution) {
    this->resolution = resolution;
    this->irradianceResolution = irradianceResolution;

    glGenTextures(1, &envTCB);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envTCB);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &irrTCB);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irrTCB);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceResolution, irradianceResolution, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &envFBO);
    glGenRenderbuffers(1, &envRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, envFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, envRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, envRBO);

    glGenFramebuffers(1, &irrFBO);
    glGenRenderbuffers(1, &irrRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, irrFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, irrRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceResolution, irradianceResolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, irrRBO);

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    Presets3D::pushVerticesCube(1.0f, vertices, indices);

    cube = new GLVertexElement<glm::vec3>(vertices, indices);
    cube->attribute({0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0});
    cube->reserve();
}

void EnvironmentMap::renderCubemap(glm::vec3 cameraPos) {
    Scene_T snapshot = scene->getSnapshot();

    snapshot.aspectRatio = 1.0f;
    snapshot.cameraCoords = {cameraPos, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
    snapshot.FOV = 90.0f;
    snapshot.width = snapshot.height = resolution;

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, snapshot.zNear, snapshot.zFar);
    static glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    Shader& batchShader = scene->getBatchShader();
    auto batchRenderTable = scene->getBatchRenderTable();
    Screenbuffer envScreen(envFBO, resolution, resolution);

    batchShader.use();
    batchShader.setMatrix4("projection", captureProjection, 1, GL_FALSE);
    batchShader.setVec3f("cameraPos", cameraPos);

    glViewport(0, 0, resolution, resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, envFBO);
    GLuint brdflut = scene->getBRDFLUT_TCB();
    if (brdflut) batchShader.setTexture("texture_brdflut", GL_TEXTURE_2D, 13, brdflut);
    glm::vec3 specLight = scene->getPBR_NonIBLRadianceGGXSpecularLight();
    glm::vec3 lambLight = scene->getPBR_NonIBLRadianceLambertianIrradiance();
    bool hasIBL = scene->isIBL_Enabled();
    scene->setPBR_NonIBLRadianceGGXSpecularLightToDirLight();
    scene->setPBR_NonIBLRadianceLambertianIrradianceToDirLight();
    scene->setIBL(false);
    for (unsigned int i = 0; i < 6; ++i) {
        batchShader.setMatrix4("view", captureViews[i], 1, GL_FALSE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envTCB, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto shadows = scene->getShadowMapper();
        if (shadows && shadows->isCreated()) shadows->pushUniforms(batchShader);
        batchRenderTable->forEach([&](const std::string& key, ShaderRenderable* renderable) {
            GameUtils::renderDV(renderable, snapshot, batchShader, envScreen);
        });
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    scene->setPBR_NonIBLRadianceGGXSpecularLight(specLight);
    scene->setPBR_NonIBLRadianceLambertianIrradiance(lambLight);
    scene->setIBL(hasIBL);

    irradianceShader.use();
    irradianceShader.setMatrix4("projection", captureProjection, 1, GL_FALSE);
    irradianceShader.setTexture("environmentMap", GL_TEXTURE_CUBE_MAP, 0, envTCB);

    glViewport(0, 0, irradianceResolution, irradianceResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, irrFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        irradianceShader.setMatrix4("view", captureViews[i], 1, GL_FALSE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irrTCB, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cube->draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
