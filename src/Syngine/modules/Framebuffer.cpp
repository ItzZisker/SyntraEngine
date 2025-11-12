#include "Framebuffer.hpp"

#include "Screenbuffer.hpp"
#include "Shader.hpp"
#include "Presets.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/utils/GameUtils.hpp"

#include <iostream>
#include <ostream>

using namespace syng;

Framebuffer::Framebuffer(Scene* scene) : scene(scene), outputShader(scene->getScreenShader()) {}
Framebuffer::Framebuffer(Scene* scene, Shader& outputShader) : scene(scene), outputShader(outputShader) {}
Framebuffer::~Framebuffer() {
    initTasks.clear();
    renderTasks.clear();
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &RBO);
    glDeleteTextures(1, &TCB);

    if (outputToParent && quad) {
        delete quad;
    }
}

void Framebuffer::create(unsigned int width_, unsigned int height_, bool outputToScreenShader) {
    if (FBO || RBO || TCB) {
        std::cerr << "ERROR::FRAMEBUFFER::Already Created" << std::endl;
        return;
    }
    GLenum TCBBaseFormat;

    switch (TCBFormat) {
        case GL_RGB:
#ifndef __EMSCRIPTEN__
        case GL_RGB16:
#endif
        case GL_RGB16F:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
            TCBBaseFormat = GL_RGB;
            break;
        case GL_RGBA:
#ifndef __EMSCRIPTEN__
        case GL_RGBA16:
#endif
        case GL_RGBA16F:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32F:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            TCBBaseFormat = GL_RGBA;
            break;
        default:
            std::cerr << "ERROR::FRAMEBUFFER::Invalid Color Buffer Format: " << TCBFormat << std::endl;
            return;
    }

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

#ifndef __EMSCRIPTEN__
    if (AA.isMultiSample()) {
        glGenTextures(1, &MS_TCB);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, MS_TCB);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, AA.getMultiSamples(), TCBFormat, width_, height_, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, MS_TCB, 0); 
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glGenFramebuffers(1, &MSOUT_FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, MSOUT_FBO);
    }
#endif
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(GL_TEXTURE_2D, 0, TCBFormat, width_, height_, 0, TCBBaseFormat, GL_UNSIGNED_BYTE, NULL);
    PresetsTexel::TextureFilter(GL_TEXTURE_2D, TCBFiltering);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TCB, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);

#ifndef __EMSCRIPTEN__
    if (AA.isMultiSample())
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, AA.getMultiSamples(), GL_DEPTH24_STENCIL8, width_, height_);
    else 
#endif
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    for (auto& func : initTasks) {
        func(this);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (outputToScreenShader) {
        Vertex2D corners[] = {
            {{-1.0f, -1.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        quad = Presets2D::newMeshQuad(corners[0], corners[1], TCB);
        quad->uploadVertices();
    }
    if (!outputShader.hasProgram()) {
        outputShader.init();
    }
    onCreate(width_, height_, outputToScreenShader, FBO);
}

void Framebuffer::create(bool outputToScreenShader) {
    Framebuffer::create(scene->getScreenWidth(), scene->getScreenHeight(), outputToScreenShader);
}

void Framebuffer::appendTCB(int attachmentIndex, unsigned int TCB, GLenum textureTarget, GLuint layer) {
    glBindTexture(textureTarget, TCB);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + attachmentIndex,
        textureTarget,
        TCB,
        layer
    );
    glBindTexture(textureTarget, 0);
}

void Framebuffer::setFallbackColor(glm::vec3 fallbackColor) {
    this->fallbackColor = fallbackColor;
}

void Framebuffer::setOutputAttachments(std::vector<GLenum> GL_attachments) {
    glDrawBuffers(GL_attachments.size(), GL_attachments.data());  
}

void Framebuffer::setAntiAliasing(AntiAliasing AA) {
    if (created) {
        if (this->AA.isFastApproximate() && AA.isFastApproximate()) {
            this->AA = AA;
        }
    } else {
        this->AA = AA;
    }
}

void Framebuffer::setTCBFormat(GLenum format) {
    if (!created) TCBFormat = format;
}

void Framebuffer::setTCBFiltering(GLenum filterType) {
    if (!created) TCBFiltering = filterType;
}

void Framebuffer::setHDR(class syng::HDR hdr) {
    this->hdr = hdr;
}

void Framebuffer::addInitTask(std::function<void(Framebuffer *)> task) {
    initTasks.push_back(task);
}

void Framebuffer::addRenderTask(std::function<void(Framebuffer *)> task) {
    renderTasks.push_back(task);
}

void Framebuffer::render(Screenbuffer screen) {
    if (!FBO || !RBO) return;

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, this->width, this->height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(fallbackColor[0], fallbackColor[1], fallbackColor[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (auto& func : renderTasks) {
        func(this);
    }
#ifndef __EMSCRIPTEN__
    if (AA.isMultiSample()) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MSOUT_FBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    if (outputToParent) {
        glViewport(0, 0, screen.getWidth(), screen.getHeight());
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        outputShader.use();
        outputShader.setTexture("screenTexture", GL_TEXTURE_2D, 0, TCB);
        outputShader.setBool("hdrEnabled", hdr.isEnabled());
        if (hdr.isEnabled()) {
            outputShader.setFloat("hdrExposure", hdr.exposure);
        }
        if (AA.isFastApproximate()) {
            float reduceMin, reduceMul, spanMax;
            AA.getFXAAVars(&reduceMin, &reduceMul, &spanMax);
            outputShader.setBool("fxaaEnabled", true);
            outputShader.setFloat("fxaaReduceMin", reduceMin);
            outputShader.setFloat("fxaaReduceMul", reduceMul);
            outputShader.setFloat("fxaaSpanMax", spanMax);
        } else {
            outputShader.setBool("fxaaEnabled", false);
        }

        quad->draw();
    }
}

Shader& Framebuffer::getOutputShader() {
    return this->outputShader;
}

AntiAliasing Framebuffer::getAntiAliasing() {
    return this->AA;
}

GLenum Framebuffer::getTCBFormat() {
    return this->TCBFormat;
}

class syng::HDR Framebuffer::getHDR() {
    return this->hdr;
}

unsigned int Framebuffer::getRBO() {
    return !this->RBO ? 0 : this->RBO;
}