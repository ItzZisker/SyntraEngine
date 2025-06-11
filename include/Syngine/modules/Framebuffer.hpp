#pragma once

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Scene.hpp"

/*
    Notes:
      - By default Framebuffer uses Renderbuffer as the depth-stencil attachment
      - By default Framebuffer uses Texture2D as the color-buffer output used by the screenshader
      - You can change these using an initTask function
      - Add rendering operations just as the main window, using renderTask functions
      - Framebuffer itself is also a Renderable which could be added to main window's rendertable
 */
class Framebuffer : public WindowRenderable
{
private:
    Scene* scene;

    std::vector<std::function<void(Framebuffer *)>> initTasks, renderTasks;
    RenderTable<ShaderRenderable>* renderTable = new RenderTable<ShaderRenderable>();

    bool outputToQuad;
    unsigned int quadVAO = 0, quadVBO = 0;

    unsigned int FBO = 0, RBO = 0, TCB = 0; // Texture Color Buffer
public:
    Framebuffer(Scene* scene);

    ~Framebuffer();

    void create(GameWindow* window, bool outputToScreenShader = true);

    void addInitTask(std::function<void(Framebuffer *)> task);

    void addRenderTask(std::function<void(Framebuffer *)> task);

    void render(int parentFBO);

    void render(GameWindow* window, int parentFBO) override {
        render(parentFBO);
    }

    RenderTable<ShaderRenderable>* getRenderTable();

    unsigned int getRBO();

    unsigned int getFBO();
};