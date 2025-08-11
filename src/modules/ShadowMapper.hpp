#pragma once

#include "Shader.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"

namespace syng
{
class ShadowMapper {
private:
    RenderTable<ShaderRenderable> *depthRendertable = new RenderTable<ShaderRenderable>();
    Shader depthShader = Shader("shaders/depthShaderVert.glsl", "shaders/depthShaderFrag.glsl");
    GLuint depthMapFBO = 0, depthMapTCB = 0;
public:
    GLfloat strength = 0.5f;
    GLfloat biasMin = 0.001f, biasMax = 0.016f;
    GLuint shadowWidth = 1024, shadowHeight = 1024;
    GLuint pcfRadius = 1;
    GLfloat pcfScale = 1.0f;
    glm::mat4 lightProjection, lightView;

    ShadowMapper(GLuint uv = 1024);
    ShadowMapper(GLuint width, GLuint height);
    ShadowMapper(GLuint width, GLuint height, glm::mat4 lightProj, glm::mat4 lightView);
    ~ShadowMapper();

    void create();
    void renderDepth(Screenbuffer screen, Scene *scene);
    void pushUniforms(Shader batchShader);

    GLuint getDepthMapFBO();
    GLuint getDepthMapTCB();

    const glm::mat4 getLightSpaceMatrix();
    RenderTable<ShaderRenderable>* getDepthRenderTable();

    bool isCreated() {
        return depthMapFBO && depthMapTCB;
    }
};
}