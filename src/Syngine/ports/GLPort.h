#ifndef __GL_PORT__
#define __GL_PORT__

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <webgl/webgl2.h>
#include <GLES3/gl3.h>

#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#else
#include <glad/glad.h>
#endif

#endif