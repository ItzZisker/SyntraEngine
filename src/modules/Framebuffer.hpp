#pragma once

#include "Shader.hpp"
#include "Syngine.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"

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
enum AA_Method {
    NONE,
    FXAA_1,
    FXAA_2,
    FXAA_4,
    MSAA_2,
    MSAA_4,
    IMPL
};

class AntiAliasing {
protected:
    AA_Method method = NONE;
    AntiAliasing() : method(IMPL) {}
public:
    AntiAliasing(AA_Method method) : method(method) {}

    bool isMultiSample() {
        return method == MSAA_2 || method == MSAA_4;
    }

    bool isFastApproximate() {
        return method == FXAA_1 || method == FXAA_2 || method == FXAA_4;
    }

    unsigned int getMultiSamples() {
        switch (method) {
            case MSAA_2: return 2;
            case MSAA_4: return 4;
            default: return 0;
        }
    }

    void getFXAAVars(float* reduceMin, float* reduceMul, float* spanMax) {
        switch (method) {
            case FXAA_1:
                *reduceMin = 1.0f / 128.0f;
                *reduceMul = 1.0f / 8.0f;
                *spanMax = 8.0f;
            break;
            case FXAA_2:
                *reduceMin = 1.0f / 256.0f;
                *reduceMul = 1.0f / 12.0f;
                *spanMax = 12.0f;
            break;
            case FXAA_4:
                *reduceMin = 1.0f / 512.0f;
                *reduceMul = 1.0f / 16.0f;
                *spanMax = 16.0f;
            break;
            default: return;
        }
    }
};

class FXAA : public AntiAliasing {
public:
    float reduceMin = 1.0 / 128.0;
    float reduceMul = 1.0 / 8.0;
    float spanMax = 8.0;
};

extern const AntiAliasing AA_OFF;
extern const AntiAliasing AA_FXAAx1;
extern const AntiAliasing AA_FXAAx2;
extern const AntiAliasing AA_FXAAx4;
extern const AntiAliasing AA_MSAAx2;
extern const AntiAliasing AA_MSAAx4;

class Framebuffer : public Screenbuffer, public WindowRenderable {
private:
    Scene* scene;
    Shader outputShader;

    std::vector<std::function<void(Framebuffer *)>> initTasks, renderTasks;
    RenderTable<ShaderRenderable>* renderTable = new RenderTable<ShaderRenderable>();
    AntiAliasing AA = AA_OFF;

    unsigned int MSOUT_FBO = 0, MS_TCB = 0;
    unsigned int quadVAO = 0, quadVBO = 0;
    unsigned int RBO = 0, TCB = 0; // Texture Color Buffer
public:
    Framebuffer(Scene* scene);

    Framebuffer(Scene* scene, Shader outputShader);

    ~Framebuffer();

    void create(bool outputToScreenShader = true);

    void create(unsigned int width, unsigned int height, bool outputToScreenShader = true);

    void setAntiAliasing(AntiAliasing AA);

    void addInitTask(std::function<void(Framebuffer *)> task);

    void addRenderTask(std::function<void(Framebuffer *)> task);

    void render(Screenbuffer screen);

    void render(GameWindow* window) override {
        render(*window);
    }

    RenderTable<ShaderRenderable>* getRenderTable();

    AntiAliasing getAntiAliasing();

    unsigned int getRBO();
};
}