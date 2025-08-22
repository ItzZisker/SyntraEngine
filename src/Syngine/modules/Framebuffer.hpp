#pragma once

#include "Shader.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"

namespace syng
{
class Framebuffer : public Screenbuffer, public WindowRenderable {
private:
    Scene* scene;
    Shader& outputShader;

    std::vector<std::function<void(Framebuffer *)>> initTasks, renderTasks;
    RenderTable<ShaderRenderable>* renderTable = new RenderTable<ShaderRenderable>();

    GLenum TCBFormat = GL_RGB;
    GLenum TCBFiltering = GL_LINEAR;
    AntiAliasing AA = AA_OFF;
    HDR HDR = HDR_OFF;

    Mesh2D* quad;
    unsigned int MSOUT_FBO = 0, MS_TCB = 0;
    unsigned int RBO = 0, TCB = 0;
public:
    Framebuffer(Scene* scene);
    Framebuffer(Scene* scene, Shader& outputShader);
    ~Framebuffer();

    void create(bool outputToScreenShader = true);
    void create(unsigned int width, unsigned int height, bool outputToScreenShader = true);

    void appendTCB(int attachmentIndex, unsigned int TCB, GLenum textureTarget = GL_TEXTURE_2D, GLuint layer = 0);

    void setOutputAttachments(std::vector<GLenum> GL_attachments);
    void setAntiAliasing(AntiAliasing AA);
    void setTCBFormat(GLenum format);
    void setTCBFiltering(GLenum filterType);
    void setHDR(class HDR hdr);

    void addInitTask(std::function<void(Framebuffer *)> task);
    void addRenderTask(std::function<void(Framebuffer *)> task);

    void render(Screenbuffer screen);
    void render(GameWindow* window) override {
        render(*window);
    }

    Shader& getOutputShader();
    RenderTable<ShaderRenderable>* getRenderTable();
    AntiAliasing getAntiAliasing();
    GLenum getTCBFormat();

    class HDR getHDR();

    unsigned int getRBO();
};
}