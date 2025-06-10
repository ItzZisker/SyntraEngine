#pragma once

#include "KEngine/engine/RenderTable.hpp"
#include "KEngine/modules/Shader.hpp"
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

    void render(int parentFBO = 0);

    void render(Shader shader, int parentFBO) override {
        render(parentFBO);
    }

    void render(GameWindow* window, int parentFBO) override {
        render(parentFBO);
    }
};