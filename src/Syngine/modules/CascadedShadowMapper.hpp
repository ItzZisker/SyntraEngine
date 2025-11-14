#pragma once

#include "Shader.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include <vector>

#define SG_CSM_CASCADE_COUNTS 2

namespace syng
{

class CascadedShadowMapper {
private:
    Scene* scene;
    RenderTable<ShaderRenderable> *depthRendertable = new RenderTable<ShaderRenderable>();
    GLuint depthMapsFBO = 0, depthMapsTCB = 0;
    Shader& depthCSMShader;
public:
    GLfloat strength = 0.5f;
    GLfloat biasMin = 0.001f, biasMax = 0.005f;
    GLuint shadowWidth = 1024, shadowHeight = 1024;
    GLfloat pcfScale = 1.0f;
    glm::vec3 lightDir;
    GLfloat cascadeBiasModifier = 0.5f;
    // Tune this parameter according to the scene
    GLfloat zMult = 10.0f;
    std::array<float, SG_CSM_CASCADE_COUNTS> cascadeLevels;

    CascadedShadowMapper(Scene *scene, Shader& depthCSMShader, GLuint uv, glm::vec3 lightDir);
    CascadedShadowMapper(Scene *scene, Shader& depthCSMShader, GLuint width, GLuint height, glm::vec3 lightDir);
    ~CascadedShadowMapper();

    void create();
    void renderDepth(Screenbuffer screen);
    void pushUniforms(Shader& batchShader);

    GLuint getDepthMapsFBO();
    GLuint getDepthMapsTCB();

    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);

    glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);
    std::vector<glm::mat4> getLightSpaceMatrices();

    RenderTable<ShaderRenderable>* getDepthRenderTable();

    bool isCreated() { return depthMapsFBO && depthMapsTCB; }
};

}