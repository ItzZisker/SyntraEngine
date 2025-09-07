#pragma once

#include "Shader.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"

#include "Syngine/engine/RenderTable.hpp"

namespace syng
{
class ShadowMapper {
private:
    RenderTable<ShaderRenderable> *depthRendertable = new RenderTable<ShaderRenderable>();
    GLuint depthMapFBO = 0, depthMapTCB = 0;
    Shader& depthShader;
public:
    GLfloat strength = 0.5f;
    GLfloat biasMin = 0.001f, biasMax = 0.016f;
    GLuint shadowWidth = 1024, shadowHeight = 1024;
    GLuint pcfRadius = 1;
    GLfloat pcfScale = 1.0f;
    glm::mat4 lightProjection, lightView;

    ShadowMapper(Shader& depthShader, GLuint uv = 1024);
    ShadowMapper(Shader& depthShader, GLuint width, GLuint height);
    ShadowMapper(Shader& depthShader, GLuint width, GLuint height, glm::mat4 lightProj, glm::mat4 lightView);
    ~ShadowMapper();

    void create();
    void renderDepth(Screenbuffer screen, Scene *scene);
    void pushUniforms(Shader& batchShader);

    GLuint getDepthMapFBO();
    GLuint getDepthMapTCB();

    const glm::mat4 getLightSpaceMatrix();
    RenderTable<ShaderRenderable>* getDepthRenderTable();

    bool isCreated() {
        return depthMapFBO && depthMapTCB;
    }
};
}