#include "Presets.hpp"
#include "Shader.hpp"
#include "modules/Mesh.hpp"
#include "modules/Model.hpp"

#include <filesystem>

using namespace syng;

void PresetsTexel::TextureParamSTR(GLenum target, GLenum param) {
    glTexParameteri(target, GL_TEXTURE_WRAP_S, param);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, param);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, param);
}

void PresetsTexel::TextureParamST(GLenum target, GLenum param) {
    glTexParameteri(target, GL_TEXTURE_WRAP_S, param);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, param);
}

void PresetsTexel::TextureFilter(GLenum target, GLenum param) {
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, param);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, param);
}

Mesh2D* Presets2D_newMeshQuad(Vertex2D corners[4], Texture texture) {
    Vertex2D topLeft, topRight, bottomLeft, bottomRight;

    float maxY = corners[0].position.y;
    float minY = corners[0].position.y;
    float maxX = corners[0].position.x;
    float minX = corners[0].position.x;

    for (int i = 1; i < 4; i++) {
        if (corners[i].position.y > maxY) maxY = corners[i].position.y;
        if (corners[i].position.y < minY) minY = corners[i].position.y;
        if (corners[i].position.x > maxX) maxX = corners[i].position.x;
        if (corners[i].position.x < minX) minX = corners[i].position.x;
    }

    for (int i = 0; i < 4; i++) {
        auto& v = corners[i];
        if (v.position.y == maxY && v.position.x == minX) topLeft = v;
        else if (v.position.y == maxY && v.position.x == maxX) topRight = v;
        else if (v.position.y == minY && v.position.x == minX) bottomLeft = v;
        else if (v.position.y == minY && v.position.x == maxX) bottomRight = v;
    }

    std::vector<Vertex2D> orderedVertices = {
        topRight,
        bottomRight,
        bottomLeft,
        topLeft
    };
    std::vector<GLuint> indices = {
        0, 1, 3,
        1, 2, 3
    };
    Mesh2D* res = new Mesh2D(orderedVertices, indices, glm::mat4(1.0f));
    res->setTexture(texture);

    return res;
}

Mesh2D* Presets2D::newMeshQuad(Vertex2D corners[4], std::string pathToTexel) {
    return Presets2D_newMeshQuad(corners, TextureFromFile(pathToTexel.c_str(), std::filesystem::current_path().string(), Texture_Diffuse));
}

Mesh2D* Presets2D::newMeshQuad(Vertex2D corners[4], GLuint TCB) {
    return Presets2D_newMeshQuad(corners, {TCB, Texture_Diffuse, ""});
}

void syng::Presets3D::pushVerticesCube(float size, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices) {
    float h = size * 0.5f;
    vertices = {
        {-h, -h, -h},
        { h, -h, -h},
        { h,  h, -h},
        {-h,  h, -h},
        {-h, -h,  h},
        { h, -h,  h},
        { h,  h,  h},
        {-h,  h,  h}
    };
    indices = {
        0, 1, 2,  2, 3, 0, // Back
        4, 5, 6,  6, 7, 4, // Front
        0, 4, 7,  7, 3, 0, // Left
        1, 5, 6,  6, 2, 1, // Right
        3, 2, 6,  6, 7, 3, // Top
        0, 1, 5,  5, 4, 0  // Bottom
    };
}