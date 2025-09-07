#pragma once

#include "Screenbuffer.hpp"
#include "Texture.hpp"
#include "GLObjects.hpp"
#include "Shader.hpp"
#include "Scene.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/serialization/DataSerializer.hpp"

#include "glm/fwd.hpp"

#include <filesystem>
#include <vector>

namespace syng
{
class Skybox : public DuplexRenderable
{
private:
    Scene *scene;
    GLVertexElement<glm::vec3> *cube;
    TextureCubemap texture;

    void load();
public:
    glm::vec3 hdrBoost = glm::vec3(1.0f);
    Shader& shader;

    Skybox(Scene* scene, Shader& skyboxShader);
    ~Skybox();

    void load(std::vector<std::filesystem::path> faces);
    void load(DataDeserializer *buffer);

    void render(Screenbuffer screen = {});
    void render(Shader& shader, Screenbuffer screen = {}) override {
        render(screen);
    }
    void render(GameWindow* window) override {
        render(*window);
    }
};
}