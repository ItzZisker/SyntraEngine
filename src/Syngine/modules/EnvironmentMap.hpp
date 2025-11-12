#pragma once

#include "GLObjects.hpp"
#include "Shader.hpp"
#include "Scene.hpp"

#include "Syngine/ports/GLPort.h"

#include "glm/fwd.hpp"

using namespace syng;

namespace syng
{

class EnvironmentMap {
private:
    Scene *scene;
    Shader& irradianceShader;

    GLVertexElement<glm::vec3> *cube;

    GLuint envFBO = 0, envRBO = 0;
    GLuint irrFBO = 0, irrRBO = 0;
    GLuint envTCB = 0, irrTCB = 0;

    int resolution = 0, irradianceResolution = 0;
public:
    EnvironmentMap(Scene *scene, Shader& irradianceShader);
    ~EnvironmentMap();

    void create(int resolution, int irradianceResolution);
    void renderCubemap(glm::vec3 cameraPos);

    GLuint getEnvironmentTCB() { return envTCB; }
    GLuint getIrradianceTCB() { return irrTCB; }
};

}