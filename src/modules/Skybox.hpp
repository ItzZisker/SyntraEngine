#pragma once

#include "Screenbuffer.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Shader.hpp"
#include "Scene.hpp"
#include <vector>
#include <string>

unsigned int loadCubemap(std::vector<std::string> faces);

class Skybox : public DuplexRenderable
{
private:
    Scene* scene;

    unsigned int cubemapTexture, cubeVAO, cubeVBO;
    std::vector<std::string> faces;
public:
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