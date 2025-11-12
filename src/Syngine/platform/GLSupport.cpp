#include "GLSupport.hpp"
#include "Syngine/ports/GLPort.h"
#include <sstream>

void syng::GLSupport::queryCapabilities() {
    vendor     = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    renderer   = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    version    = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLint i = 0; i < numExtensions; ++i) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (!ext) continue;
        extensions += ext + std::string(" ");
    }

    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxFragmentTextures);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextures);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertexTextures);

    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
    supportsMRT = (maxDrawBuffers >= 2 && maxColorAttachments >= 2);

    if (majorVersion > 4 || (majorVersion == 4 && minorVersion >= 3)) {
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSSBOBindings);
        supportsSSBO = (maxSSBOBindings > 0);
    } else {
        maxSSBOBindings = 0;
        supportsSSBO = false;
    }

    if (majorVersion > 3 || (majorVersion == 3 && minorVersion >= 1)) {
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUBOBindings);
        supportsUBO = (maxUBOBindings > 0);
    } else {
        maxUBOBindings = 0;
        supportsUBO = false;
    }
}

std::stringstream syng::GLSupport::getSummary() const {
    std::stringstream str;
    str << "===== OpenGL GPU Capability Report =====\n";
    str << "Vendor:   " << vendor << "\n";
    str << "Renderer: " << renderer << "\n";
    str << "Version:  " << version << " (" << majorVersion << "." << minorVersion << ")\n";
    str << "Extensions: " << extensions << "\n\n";
    str << "-- Texture Limits --\n";
    str << "Max Fragment Shader Textures: " << maxFragmentTextures << "\n";
    str << "Max Vertex Shader Textures:   " << maxVertexTextures << "\n";
    str << "Max Combined Texture Units:   " << maxCombinedTextures << "\n\n";
    str << "-- MRT (Multiple Render Targets) --\n";
    str << "Max Draw Buffers:       " << maxDrawBuffers << "\n";
    str << "Max Color Attachments:  " << maxColorAttachments << "\n";
    str << "Supports MRT:           " << (supportsMRT ? "Yes" : "No") << "\n\n";
    str << "-- Buffer Object Limits --\n";
    str << "Max SSBO Bindings: " << maxSSBOBindings << "  ("
              << (supportsSSBO ? "Supported" : "Not Supported") << ")\n";
    str << "Max UBO Bindings:  " << maxUBOBindings << "  ("
              << (supportsUBO ? "Supported" : "Not Supported") << ")\n";
    str << "========================================\n";
    return str;
}
