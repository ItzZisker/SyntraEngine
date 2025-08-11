#pragma once

#include "Screenbuffer.hpp"
#include "engine/RenderTable.hpp"
#include "glm/fwd.hpp"
#include "modules/GLObjects.hpp"
#include "modules/Shader.hpp"
#include "Scene.hpp"
#include <vector>
#include <string>

namespace syng
{
unsigned int loadCubemap(std::vector<std::string> faces);

class Skybox : public DuplexRenderable
{
private:
    Scene* scene;
    GLVertexElement<glm::vec3>* cube;
    std::vector<std::string> faces;
    GLuint cubemapTexture;
public:
    glm::vec3 hdrBoost = glm::vec3(1.0f);

    Shader shader = Shader("shaders/skyboxVertex.glsl", "shaders/skyboxFrag.glsl");

    Skybox(Scene* scene, std::vector<std::string> faces);
    ~Skybox();

    void load();
    void render(Screenbuffer screen = {});

    void render(Shader shader, Screenbuffer screen = {}) override {
        render(screen);
    }

    void render(GameWindow* window) override {
        render(*window);
    }
};
}