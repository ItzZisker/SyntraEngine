#include "Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include <modules/Screenbuffer.hpp>

using namespace syng;

namespace syng {
    const HDR HDR_OFF = {0.0f};

    const AntiAliasing AA_OFF = {NONE};
    const AntiAliasing AA_FXAAx1 = {FXAA_1};
    const AntiAliasing AA_FXAAx2 = {FXAA_2};
    const AntiAliasing AA_FXAAx4 = {FXAA_4};
    const AntiAliasing AA_MSAAx2 = {MSAA_2};
    const AntiAliasing AA_MSAAx4 = {MSAA_4};
}

Screenbuffer::Screenbuffer(GLuint FBO) : FBO(FBO) {}

Screenbuffer::Screenbuffer(GLuint FBO, GLuint width, GLuint height, bool outputToParent)
                            : FBO(FBO), width(width), height(height), outputToParent(outputToParent) {}

void Screenbuffer::onCreate(GLuint width, GLuint height, bool outputToParent, GLuint FBO) {
    this->width = width;
    this->height = height;
    this->outputToParent = outputToParent;
    this->FBO = FBO;
    SetupObject::onCreate();
}

GLuint Screenbuffer::getFBO() {
    return this->FBO;
}

GLuint Screenbuffer::getWidth() {
    return this->width;
}

GLuint Screenbuffer::getHeight() {
    return this->height;
}

bool Screenbuffer::isOutputToParent() {
    return this->outputToParent;
}

void Screenbuffer::bind(GLenum target) {
    glBindFramebuffer(target, FBO);
}