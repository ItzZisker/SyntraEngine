#pragma once

#include "Shader.hpp"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Scene.hpp"
#include "modules/Shader.hpp"

/*
    Notes:
      - By default Framebuffer uses Renderbuffer as the depth-stencil attachment
      - By default Framebuffer uses Texture2D as the color-buffer output used by the screenshader
      - You can change these using an initTask function
      - Add rendering operations just as the main window, using renderTask functions
      - Framebuffer itself is also a Renderable which could be added to main window's rendertable
 */
namespace syng
{
class Framebuffer : public Screenbuffer, public WindowRenderable {
private:
    Scene* scene;
    Shader outputShader;

    std::vector<std::function<void(Framebuffer *)>> initTasks, renderTasks;
    RenderTable<ShaderRenderable>* renderTable = new RenderTable<ShaderRenderable>();
    GLenum TCBFormat = GL_RGB;
    AntiAliasing AA = AA_OFF;
    HDR HDR = HDR_OFF;

    unsigned int MSOUT_FBO = 0, MS_TCB = 0;
    unsigned int quadVAO = 0, quadVBO = 0;
    unsigned int RBO = 0, TCB = 0; // Texture Color Buffer
public:
    Framebuffer(Scene* scene);

    Framebuffer(Scene* scene, Shader outputShader);

    ~Framebuffer();

    void create(bool outputToScreenShader = true);

    void create(unsigned int width, unsigned int height, bool outputToScreenShader = true);

    void appendTCB(int attachmentIndex, unsigned int TCB, GLenum textureTarget = GL_TEXTURE_2D, GLuint layer = 0);

    void setOutputAttachments(std::vector<GLenum> GL_attachments);

    void setAntiAliasing(AntiAliasing AA);

    void setTCBFormat(GLenum format);

    void setHDR(class HDR hdr);

    void addInitTask(std::function<void(Framebuffer *)> task);

    void addRenderTask(std::function<void(Framebuffer *)> task);

    void render(Screenbuffer screen);

    void render(GameWindow* window) override {
        render(*window);
    }

    RenderTable<ShaderRenderable>* getRenderTable();

    Shader getOutputShader();

    AntiAliasing getAntiAliasing();

    GLenum getTCBFormat();

    class HDR getHDR();

    unsigned int getRBO();
};
}