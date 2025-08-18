#pragma once

#include <glad/glad.h>

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

class HDR {
public:
    float exposure = 3.25f;

    bool isEnabled() {
        return exposure != 0.0f;
    }
};

extern const HDR HDR_OFF;

extern const AntiAliasing AA_OFF;
extern const AntiAliasing AA_FXAAx1;
extern const AntiAliasing AA_FXAAx2;
extern const AntiAliasing AA_FXAAx4;
extern const AntiAliasing AA_MSAAx2;
extern const AntiAliasing AA_MSAAx4;

class SetupObject {
protected:
    bool created = false;

    void onCreate() {
        this->created = true;
    }
public:
    bool isCreated() {
        return this->created;
    }
};

class Screenbuffer : public SetupObject {
protected:
    GLuint FBO = 0;
    GLuint width = 0, height = 0;
    bool outputToParent = false;

    void onCreate(GLuint width, GLuint height, bool outputToParent, GLuint FBO = 0);
public:
    Screenbuffer(GLuint FBO = 0);
    Screenbuffer(GLuint FBO, GLuint width, GLuint height, bool outputToParent = false);

    void bind(GLenum target = GL_FRAMEBUFFER);

    GLuint getFBO();
    GLuint getWidth();
    GLuint getHeight();
    bool isOutputToParent();
};
}