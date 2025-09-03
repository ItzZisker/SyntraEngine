#pragma once

#include "Scene.hpp"
#include "Shader.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <functional>

#define SG_CUBEMAP_SIDES 6

namespace syng
{
class CubemapFramebuffer : public Screenbuffer, public DuplexRenderable {
public:
    int sceneSize = 512;
    float fieldOfView = 89.527f, aspectRatio = 1.0f;
    float zNear = 0.1f, zFar = 100.0f;

    CubemapFramebuffer(Scene* scene, Shader &reflectionShader, Shader &refractionShader);
    ~CubemapFramebuffer();

    Scene_T getSnapshot(Coordination cubemapSideView);

    void create(bool renderToParent = true);

    void render(Screenbuffer screen);
    void render(GameWindow* window) override {
        render(*window);
    }
    void render(Shader& window, Screenbuffer screen) override {
        render(screen);
    }

    void addInitTask(std::function<void(CubemapFramebuffer*)> task);
    void addRenderTask(std::function<void(unsigned int FBO, const Coordination& sideView)> task);

    RenderTable<ShaderRenderable>* getReflectionRenderTable();
    RenderTable<ShaderRenderable>* getRefractionRenderTable();

    unsigned int getCubemapTexture() const;
    const unsigned int* getFBOs() const;
    const unsigned int* getRBOs() const;
private:
    Shader &reflectionShader, &refractionShader;
    Scene* scene;

    unsigned int FBO[SG_CUBEMAP_SIDES] = {0, 0, 0, 0, 0, 0};
    unsigned int RBO[SG_CUBEMAP_SIDES] = {0, 0, 0, 0, 0, 0};
    unsigned int cubemapTexture = 0;

    RenderTable<ShaderRenderable> *reflectionRendertable = new RenderTable<ShaderRenderable>();
    RenderTable<ShaderRenderable> *refractionRendertable = new RenderTable<ShaderRenderable>();

    std::vector<std::function<void(CubemapFramebuffer* cubemapFramebuffer)>> initTasks;
    std::vector<std::function<void(unsigned int FBO, const Coordination& sideView)>> renderTasks;

    void renderDV(ShaderRenderable* renderable, Shader shader, int parentFBO);
    void renderCubemap(ShaderRenderable* renderable, int parentFBO);
    
    void createFramebuffer(int index);
};
}