#pragma once

#include "Mesh.hpp"

namespace syng
{
namespace PresetsTexel
{
    void TextureParamSTR(GLenum target = GL_TEXTURE_2D, GLenum param = GL_REPEAT);
    void TextureParamST(GLenum target = GL_TEXTURE_2D, GLenum param = GL_REPEAT);
    void TextureFilter(GLenum target = GL_TEXTURE_2D, GLenum param = GL_LINEAR);
}

namespace Presets2D
{
    Mesh2D* newMeshQuad(Vertex2D corners[4], std::string pathToTexel = "");
    Mesh2D* newMeshQuad(Vertex2D corners[4], GLuint TCB);
}

namespace Presets3D
{
    void pushVerticesCube(float size, std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices);
}

}