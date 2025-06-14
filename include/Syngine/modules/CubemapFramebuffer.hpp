#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>

#define SG_CUBEMAP_SIDES 6

/*
    Notes:
      - CubemapFramebuffer renders the scene into a dynamic cubemap (6 faces)
      - It uses a TextureCubemap as color output and a single Renderbuffer as the shared depth buffer
      - The cubemap can be used for dynamic reflection or refraction
      - Use addInitTask to customize initialization
      - Use addRenderTask to define scene rendering per face (view matrix provided)
      - You can call renderToCubemap() each frame to update the environment map
 */
class CubemapFramebuffer : public DuplexRenderable {
public:
    int sceneSize = 512;
    float fieldOfView = 89.46666f, aspectRatio = 1.0f;
    float zNear = 0.1f, zFar = 100.0f;

    CubemapFramebuffer(Scene* scene);

    ~CubemapFramebuffer();

    void create(bool renderToParent = true);

    void render(int parentFBO);

    void render(GameWindow* window, int parentFBO) override {
        render(parentFBO);
    }

    void render(Shader window, int parentFBO) override {
        render(parentFBO);
    }

    void addInitTask(std::function<void(CubemapFramebuffer*)> task);

    void addRenderTask(std::function<void(unsigned int, const glm::mat4& view)> task);

    RenderTable<ShaderRenderable>* getReflectionRenderTable();

    RenderTable<ShaderRenderable>* getRefractionRenderTable();

    unsigned int getCubemapTexture() const;

    const unsigned int* getFBOs() const;

    const unsigned int* getRBOs() const;
private:
    bool renderToParent;

    Shader reflectionShader = Shader("shaders/reflectionVertex.glsl", "shaders/reflectionFrag.glsl");
    Shader refractionShader = Shader("shaders/refractionVertex.glsl", "shaders/refractionFrag.glsl");

    Scene* scene;

    unsigned int FBO[SG_CUBEMAP_SIDES] = {0, 0, 0, 0, 0, 0};
    unsigned int RBO[SG_CUBEMAP_SIDES] = {0, 0, 0, 0, 0, 0};
    unsigned int cubemapTexture = 0;

    RenderTable<ShaderRenderable> *reflectionRendertable = new RenderTable<ShaderRenderable>();
    RenderTable<ShaderRenderable> *refractionRendertable = new RenderTable<ShaderRenderable>();

    std::vector<std::function<void(CubemapFramebuffer*)>> initTasks;
    std::vector<std::function<void(unsigned int, const glm::mat4& view)>> renderTasks;

    void renderCubemap(ShaderRenderable* renderable, int parentFBO);

    void createFramebuffer(int index);
};