#pragma once

#include <Syngine/ports/GLPort.h>
#include <sstream>
#include <string>

namespace syng
{
class GLSupport {
public:
    std::string extensions;
    std::string vendor;
    std::string renderer;
    std::string version;

    int majorVersion = 0;
    int minorVersion = 0;

    int maxFragmentTextures = 0;
    int maxCombinedTextures = 0;
    int maxVertexTextures = 0;
    int maxDrawBuffers = 0;
    int maxColorAttachments = 0;
    int maxSSBOBindings = 0;
    int maxUBOBindings = 0;

    bool supportsMRT = false;
    bool supportsSSBO = false;
    bool supportsUBO = false;
public:
    GLSupport() = default;

    void queryCapabilities();
    std::stringstream getSummary() const;

    bool hasMRT() const { return supportsMRT; }
    bool hasSSBO() const { return supportsSSBO; }
    bool hasUBO() const { return supportsUBO; }
};
}