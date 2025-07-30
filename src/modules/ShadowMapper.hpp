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
    unsigned int depthMapFBO = 0, depthMapTCB = 0;
public:
    float strength = 0.5f;
    float biasMin = 0.001f, biasMax = 0.016f;
    unsigned int shadowWidth = 1024, shadowHeight = 1024;
    glm::mat4 lightProjection, lightView;

    ShadowMapper(unsigned int uv = 1024);

    ShadowMapper(unsigned int width, unsigned int height);

    ShadowMapper(unsigned int width, unsigned int height, glm::mat4 lightProj, glm::mat4 lightView);

    ~ShadowMapper();

    void create();

    bool isCreated() {
        return depthMapFBO && depthMapTCB;
    }

    void renderDepth(Screenbuffer screen, Scene *scene);

    unsigned int getDepthMapFBO();

    unsigned int getDepthMapTCB();

    const glm::mat4 getLightSpaceMatrix();

    RenderTable<ShaderRenderable>* getDepthRenderTable();
};
}