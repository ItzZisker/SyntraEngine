#include "Presets.hpp"
#include "modules/GLObjects.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include <modules/Skybox.hpp>

#include <iostream>
#include <ostream>
#include <stb_image.h>
#include <glad/glad.h>
#include <vector>

using namespace syng;

unsigned int syng::loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    int width, height, nrChannels;

    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
            width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "ERROR::SKYBOX::loadCubemap() failed at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    PresetsTexel::TextureFilter(GL_TEXTURE_CUBE_MAP);
    PresetsTexel::TextureParamSTR(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE);

    return textureID;
}

Skybox::Skybox(Scene* scene, std::vector<std::string> faces)
    : scene(scene), faces(faces) {}

Skybox::~Skybox() {
    faces.clear();
    shader.disposeProgram();
    delete cube;
}

void Skybox::load() {
    shader.init();
    cubemapTexture = loadCubemap(faces);

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    Presets3D::pushVerticesCube(2.0f, vertices, indices);

    cube = new GLVertexElement<glm::vec3>(vertices, indices);
    cube->attribute({0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0});
    cube->reserve();
}

void Skybox::render(Screenbuffer screen) {
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    shader.use();
    shader.setMatrix4("view", glm::mat4(glm::mat3(scene->getCamera()->getViewMatrix())), 1, GL_FALSE);
    shader.setMatrix4("projection", scene->getProjection(), 1, GL_FALSE);
    shader.setTexture("skybox", GL_TEXTURE_CUBE_MAP, 0, cubemapTexture);
    shader.setVec3f("hdrBoost", hdrBoost);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    cube->draw();

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}