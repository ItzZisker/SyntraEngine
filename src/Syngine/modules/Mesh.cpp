#include "Mesh.hpp"

#include "Shader.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "GLObjects.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"

#include "glm/fwd.hpp"

#include <string>
#include <vector>

using namespace syng;

Mesh::Mesh(int meshID, std::vector<Vertex> vertices, std::vector<unsigned int> indices) 
    : GLVertexElement<Vertex>(vertices, indices), meshID(meshID) {}

void Mesh::setMaterial(Material *mat) {
    this->material = mat;
}

Material* Mesh::getMaterial() {
    return this->material;
}

int Mesh::getID() {
    return this->meshID;
}

void Mesh::uploadVertices(CacheApproach::VRAM_Approach approach) {
    if (isLoaded()) return;

    switch (approach) {
        case CacheApproach::Sequential:
            attribute({0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0});
            attribute({1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal)});
            attribute({2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords)});
            attribute({3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent)});
            attribute({4, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, m_BoneIDs), GLPointer_Int32});
            attribute({5, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, m_Weights)});
            reserve();
        break;
        case CacheApproach::Interleaved:        
            size_t count = vertices.size();
        
            int vec2fLength   = 2 * count;
            int vec3fLength   = 3 * count;
            int boneLength    = MAX_BONE_INFLUENCE * count;

            float* positions   = new float[vec3fLength];
            float* normals     = new float[vec3fLength];
            float* texCoords   = new float[vec2fLength];
            float* tangents    = new float[vec3fLength];
            float* m_Weights   = new float[boneLength];
            int* m_BoneIDs     = new int[boneLength];

            for (size_t i = 0; i < count; ++i) {
                const Vertex& v = vertices[i];

                positions[i * 3 + 0] = v.position.x;
                positions[i * 3 + 1] = v.position.y;
                positions[i * 3 + 2] = v.position.z;

                normals[i * 3 + 0] = v.normal.x;
                normals[i * 3 + 1] = v.normal.y;
                normals[i * 3 + 2] = v.normal.z;

                texCoords[i * 2 + 0] = v.texCoords.x;
                texCoords[i * 2 + 1] = v.texCoords.y;

                tangents[i * 3 + 0] = v.tangent.x;
                tangents[i * 3 + 1] = v.tangent.y;
                tangents[i * 3 + 2] = v.tangent.z;

                for (int j = 0; j < MAX_BONE_INFLUENCE; ++j) {
                    m_BoneIDs[i * MAX_BONE_INFLUENCE + j] = v.m_BoneIDs[j];
                    m_Weights[i * MAX_BONE_INFLUENCE + j] = v.m_Weights[j];
                }
            }

            GLintptr offset = 0;

            dataSub({offset, static_cast<GLsizeiptr>(vec3fLength * sizeof(float)), positions});
            offset += vec3fLength * sizeof(float);
            dataSub({offset, static_cast<GLsizeiptr>(vec3fLength * sizeof(float)), normals});
            offset += vec3fLength * sizeof(float);
            dataSub({offset, static_cast<GLsizeiptr>(vec2fLength * sizeof(float)), texCoords});
            offset += vec2fLength * sizeof(float);
            dataSub({offset, static_cast<GLsizeiptr>(vec3fLength * sizeof(float)), tangents});
            offset += vec3fLength * sizeof(float);
            dataSub({offset, static_cast<GLsizeiptr>(boneLength * sizeof(int)), m_BoneIDs});
            offset += boneLength * sizeof(int);
            dataSub({offset, static_cast<GLsizeiptr>(boneLength * sizeof(float)), m_Weights});

            GLuint attribIndex = 0;
            offset = 0;

            attribute({attribIndex++, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});
            offset += vec3fLength * sizeof(float);
            attribute({attribIndex++, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});
            offset += vec3fLength * sizeof(float);
            attribute({attribIndex++, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});
            offset += vec2fLength * sizeof(float);
            attribute({attribIndex++, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});
            offset += vec3fLength * sizeof(float);
            attribute({attribIndex++, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, 0, (void*)(offset), GLPointer_Int32});
            offset += boneLength * sizeof(int);
            attribute({attribIndex++, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});

            reserve();

            delete[] positions;
            delete[] normals;
            delete[] texCoords;
            delete[] tangents;
            delete[] m_Weights;
            delete[] m_BoneIDs;
        break;
    }
}

Mesh2D::Mesh2D(std::vector<Vertex2D> vertices, std::vector<GLuint> indices) : GLVertexElement<Vertex2D>(vertices, indices) {}

Mesh2D::~Mesh2D() {
    if (texture_fallback) glDeleteTextures(1, &texture_fallback);
    GLuint TCB = meshTexture.getTCB();
    if (TCB) glDeleteTextures(1, &TCB);
}

void Mesh2D::setFallbackTCB(GLuint TCB) {
    if (!TCB) return;
    if (texture_fallback) glDeleteTextures(1, &texture_fallback);
    this->texture_fallback = TCB;
}

void Mesh2D::setFallbackColor(GLubyte pixel[4]) {
    GLuint TCB = 0;
    TCBPlainColor(TCB, pixel);
    setFallbackTCB(TCB);
}

void Mesh2D::setTexture(Texture2D texel) {
    if (!texel.getTCB()) return;
    GLuint TCB = meshTexture.getTCB();
    if (TCB) glDeleteTextures(1, &TCB);
    this->meshTexture = texel;
}

void Mesh2D::render(Shader& shader, Screenbuffer screen, glm::mat4 transform) { // TODO: Create 2D Batch-Renderer
    if (!isLoaded()) return;
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    if (!meshTexture.getTCB() && !texture_fallback) {
        GLubyte pixel[4] = {0, 128, 128, 255};
        setFallbackColor(pixel);
    }
    shader.use();
    shader.setMatrix4("model", transform, 1, GL_FALSE);
    shader.setTexture("texture2D", GL_TEXTURE_2D, 0, meshTexture.getTCB() ? meshTexture.getTCB() : texture_fallback);
    
    GLVertexElement<Vertex2D>::draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Mesh2D::uploadVertices(CacheApproach::VRAM_Approach approach) {
    if (isLoaded()) return;

    switch (approach) {
        case CacheApproach::Sequential:
            attribute({0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, position)});
            attribute({1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, texCoords)});
            reserve();
        break;
        case CacheApproach::Interleaved:
            size_t count = vertices.size();

            int vec2fLength = 2 * count;

            float* positions = new float[vec2fLength];
            float* texcoords = new float[vec2fLength];

            for (size_t i = 0; i < count; ++i) {
                const Vertex2D& v = vertices[i];

                positions[i * 2 + 0] = v.position.x;
                positions[i * 2 + 1] = v.position.y;

                texcoords[i * 2 + 0] = v.texCoords.x;
                texcoords[i * 2 + 1] = v.texCoords.y;
            }

            GLintptr offset = 0;
            dataSub({offset, static_cast<GLsizeiptr>(vec2fLength * sizeof(float)), positions});
            offset += vec2fLength * sizeof(float);
            dataSub({offset, static_cast<GLsizeiptr>(vec2fLength * sizeof(float)), texcoords});

            GLuint attribIndex = 0;
            offset = 0;

            attribute({attribIndex++, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});
            offset += vec2fLength * sizeof(float);
            attribute({attribIndex++, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset)});

            reserve();

            delete[] positions;
            delete[] texcoords;
        break;
    }
}