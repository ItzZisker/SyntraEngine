#include "Presets.hpp"
#include "Texture.hpp"

#include "Syngine/modules/Mesh.hpp"

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

Mesh2D* Presets2D_newMeshQuad(Vertex2D min, Vertex2D max, Texture2D texture) {
    Vertex2D topRight = {{max.position.x, max.position.y}, {max.texCoords.x, max.texCoords.y}};
    Vertex2D bottomRight = {{max.position.x, min.position.y}, {max.texCoords.x, min.texCoords.y}};
    Vertex2D bottomLeft = {{min.position.x, min.position.y}, {min.texCoords.x, min.texCoords.y}};
    Vertex2D topLeft = {{min.position.x, max.position.y}, {min.texCoords.x, max.texCoords.y}};

    std::vector<Vertex2D> orderedVertices = {
        topRight,
        bottomRight,
        bottomLeft,
        topLeft
    };
    std::vector<GLuint> indices = {
        2, 1, 3,
        1, 0, 3
    };
    Mesh2D* res = new Mesh2D(orderedVertices, indices, glm::mat4(1.0f));
    res->setTexture(texture);

    return res;
}

Mesh2D* Presets2D::newMeshQuad(Vertex2D min, Vertex2D max, std::string pathToTexel) {
    return Presets2D_newMeshQuad(min, max, loadTexture2D(pathToTexel.c_str()));
}

Mesh2D* Presets2D::newMeshQuad(Vertex2D min, Vertex2D max, GLuint TCB) {
    return Presets2D_newMeshQuad(min, max, {TCB, ""});
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
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        0, 4, 7,  7, 3, 0,
        1, 5, 6,  6, 2, 1,
        3, 2, 6,  6, 7, 3,
        0, 1, 5,  5, 4, 0
    };
}