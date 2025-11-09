#include "Skybox.hpp"

#include "Presets.hpp"
#include "Texture.hpp"

#include "Syngine/ports/GLPort.h"

#include <filesystem>
#include <vector>

using namespace syng;

Skybox::Skybox(Scene* scene, Shader& skyboxShader) : scene(scene), shader(skyboxShader) {}

Skybox::~Skybox() {
    delete cube;
}

void Skybox::load() {
    shader.init();
    shader.use();
    shader.setVec3f("hdrBoost", hdrBoost);

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    Presets3D::pushVerticesCube(2.0f, vertices, indices);

    cube = new GLVertexElement<glm::vec3>(vertices, indices);
    cube->attribute({0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0});
    cube->reserve();
}

void Skybox::load(DataDeserializer *buffer) {
    texture = TextureIO::TexturePackedReader(buffer).readTextureCubemap();
    texture.uploadTexture();
    Skybox::load();
}

void Skybox::load(std::vector<std::filesystem::path> paths) {
    texture = TextureIO::TextureFileReader::readTextureCubemap(paths);
    texture.uploadTexture();
    Skybox::load();
}

void Skybox::render(Screenbuffer screen) {
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    shader.use();
    shader.setMatrix4("view", glm::mat4(glm::mat3(scene->getCamera()->getViewMatrix())), 1, GL_FALSE);
    shader.setMatrix4("projection", scene->getProjection(), 1, GL_FALSE);
    shader.setTexture("skybox", GL_TEXTURE_CUBE_MAP, 0, texture.getTCB());
    shader.setVec3f("hdrBoost", hdrBoost);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    cube->draw();

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}