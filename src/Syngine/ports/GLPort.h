#ifndef __GL_PORT__
#define __GL_PORT__

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <webgl/webgl2.h>
#include <GLES3/gl3.h>

#define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#else
#include <glad/glad.h>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3

#endif

#endif