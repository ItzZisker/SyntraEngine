#include "Mesh.hpp"

#include "Presets.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "GLObjects.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"

#include "Syngine/engine/Config.hpp"
#include "Syngine/world/Coordination.hpp"

#include "glm/fwd.hpp"

#include <string>
#include <vector>

using namespace syng;

void createPlainTexture(unsigned int &TCB, unsigned char pixel[4]) {
    glGenTextures(1, &TCB);
    glBindTexture(GL_TEXTURE_2D, TCB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    PresetsTexel::TextureFilter(GL_TEXTURE_2D, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

M_Metadata::M_Metadata(int type, const char *data, int data_length)
    : type(type), data_length(data_length) {
    data_copy.resize(data_length);
    if (data && data_length > 0) {
        std::memcpy(data_copy.data(), data, data_length);
    }
}

int M_Metadata::dataLength() {
    return data_length;
}

std::vector<char>& M_Metadata::rawData() {
    return data_copy;
}

M_Metadata::Value M_Metadata::getValue() const {
    const char *raw = data_copy.data();

    switch (type) {
        case 0x1: { // Float
            float val{};
            if (data_length >= sizeof(float)) {
                std::memcpy(&val, raw, sizeof(float));
            }
            return val;
        }
        case 0x2: { // Double
            double val{};
            if (data_length >= sizeof(double)) {
                std::memcpy(&val, raw, sizeof(double));
            }
            return val;
        }
        case 0x3: { // String
            return std::string(raw, data_length);
        }
        case 0x4: { // Integer
            int val{};
            if (data_length >= sizeof(int)) {
                std::memcpy(&val, raw, sizeof(int));
            }
            return val;
        }
        case 0x5: { // Buffer
            return data_copy;
        }
        default:
            throw std::runtime_error("Unknown metadata type");
    }
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, glm::mat4 parenToNodeTransform)
    : GLVertexElement<Vertex>(vertices, indices), parentToNodeTransform(parenToNodeTransform) {}

Mesh::~Mesh() {
    vertices.clear();
    indices.clear();

    for (auto& mT : textures) {
        glDeleteTextures(1, &mT.texture.TCB);
    }
    textures.clear();

    for (auto& pair : textures_fallback) {
        glDeleteTextures(1, &pair.second);
    }
}

glm::mat4 Mesh::getParentToNodeTransform() {
    return this->parentToNodeTransform;
}

Coordination Mesh::getParentToNodeCoords() {
    return {getParentToNodeTransform()};
}

int Mesh::getMaterialId() {
    return this->material->getID();
}

bool Mesh::hasFallback(MeshTexture2D_T texType) {
    return textures_fallback.find(texType) != textures_fallback.end();
}

void Mesh::setFallbackTCB(MeshTexture2D_T texType, GLuint TCB) {
    auto last = textures_fallback.find(texType);
    if (last != textures_fallback.end()) {
        glDeleteTextures(1, &(last->second));
        textures_fallback.erase(last);
    }
    textures_fallback.emplace(texType, TCB);
}

void Mesh::setFallbackColor(MeshTexture2D_T texType, GLubyte pixel[4]) {
    GLuint TCB = 0;
    createPlainTexture(TCB, pixel);
    setFallbackTCB(texType, TCB);
}

void Mesh::render(Shader& shader, Screenbuffer screen, glm::mat4 transform) {
    if (!isLoaded()) return;
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    shader.use();
    shader.setMatrix4("model", transform, 1, GL_FALSE);
    shader.setFloat("F0", material->props.F0);
    shader.setVec3f("ior", material->props.ior);
    shader.setFloat("shininess", material->props.shininess);

    if (shader.getVariable(SHADER_REFRAC_KEY_DYNAMIC_OPACITY) == SHADER_VAL_ON) {
        shader.setFloat("minOpacity", material->props.minOpacity);
        shader.setFloat("maxOpacity", material->props.maxOpacity);
    } else {
        shader.setFloat("opacity", material->props.opacity);
    }

    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int roughNr = 1;
    unsigned int texUnit = 0;

    if (textures.empty() && !hasFallback(Texture_Diffuse)) {
        unsigned char white[4] = {255, 255, 255, 255};
        setFallbackColor(Texture_Diffuse, white);
    }
    if (textures.empty() && hasFallback(Texture_Diffuse)) {
        shader.setTexture("texture_diffuse1", GL_TEXTURE_2D, texUnit++, textures_fallback[Texture_Diffuse]);
    }

    for (const auto& tex : textures) {
        std::string number;

        switch (tex.type) {
            case Texture_Diffuse: number = std::to_string(diffuseNr++); break;
            case Texture_Specular: number = std::to_string(specularNr++); break;
            case Texture_Normal: number = std::to_string(normalNr++); break;
            case Texture_Height: number = std::to_string(heightNr++); break;
            case Texture_Rough: number = std::to_string(roughNr++); break;
        }

        shader.setTexture(TEXTURE_NAME(tex.type) + number, GL_TEXTURE_2D, texUnit++, tex.texture.TCB);
    }
    
    if (specularNr == 1 && !hasFallback(Texture_Specular)) {
        unsigned char zC[3] = {255, 255, 255};
        setFallbackColor(Texture_Specular, zC);
    }
    if (specularNr == 1 && hasFallback(Texture_Specular)) {
        shader.setTexture("texture_specular1", GL_TEXTURE_2D, texUnit++, textures_fallback[Texture_Specular]);
    }
    if (normalNr == 1 && !hasFallback(Texture_Normal)) {
        unsigned char zC[3] = {128, 128, 255};
        setFallbackColor(Texture_Normal, zC);
    }
    if (normalNr == 1 && hasFallback(Texture_Normal)) {
        shader.setTexture("texture_normal1", GL_TEXTURE_2D, texUnit++, textures_fallback[Texture_Normal]);
    }

    shader.setBool("parallax", heightNr > 1  && material->props.hasDisplacement);
    shader.setBool("roughness", roughNr > 1 && material->props.hasRoughness);

    GLVertexElement::draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Mesh::init(CacheApproach::VRAM_Approach approach) {
    if (isLoaded()) return;

    switch (approach) {
        case CacheApproach::Sequential:
            attribute({0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0});
            attribute({1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal)});
            attribute({2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords)});
            attribute({3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent)});
            attribute({4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, bitangent)});
            attribute({5, MAX_BONE_INFLUENCE, GL_INT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, m_BoneIDs), GLPointer_Int32});
            attribute({6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, m_Weights)});
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
            float* biTangents  = new float[vec3fLength];
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

                biTangents[i * 3 + 0] = v.bitangent.x;
                biTangents[i * 3 + 1] = v.bitangent.y;
                biTangents[i * 3 + 2] = v.bitangent.z;

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
            dataSub({offset, static_cast<GLsizeiptr>(vec3fLength * sizeof(float)), biTangents});
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
            delete[] biTangents;
            delete[] m_Weights;
            delete[] m_BoneIDs;
        break;
    }
}

Mesh2D::Mesh2D(std::vector<Vertex2D> vertices, std::vector<GLuint> indices, glm::mat4 parentToNodeTransform) : GLVertexElement<Vertex2D>(vertices, indices) {
    this->parentToNodeTransform = parentToNodeTransform;
}

Mesh2D::~Mesh2D() {
    if (texture_fallback) glDeleteTextures(1, &texture_fallback);
    if (meshTexture.texture.TCB) glDeleteTextures(1, &meshTexture.texture.TCB);
}

glm::mat4 Mesh2D::getParentToNodeTransform() {
    return this->parentToNodeTransform;
}

Coordination Mesh2D::getParentToNodeCoords() {
    return Coordination(getParentToNodeTransform());
}

void Mesh2D::setFallbackTCB(GLuint TCB) {
    if (!TCB) return;
    if (texture_fallback) glDeleteTextures(1, &texture_fallback);
    this->texture_fallback = TCB;
}

void Mesh2D::setFallbackColor(GLubyte pixel[4]) {
    GLuint TCB = 0;
    createPlainTexture(TCB, pixel);
    setFallbackTCB(TCB);
}

void Mesh2D::setTexture(MeshTexture2D mT) {
    if (!mT.texture.TCB) return;
    if (this->meshTexture.texture.TCB) glDeleteTextures(1, &this->meshTexture.texture.TCB);
    this->meshTexture = mT;
}

void Mesh2D::render(Shader& shader, Screenbuffer screen, glm::mat4 transform) {
    if (!isLoaded()) return;
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    if (!meshTexture.texture.TCB && !texture_fallback) {
        GLubyte pixel[4] = {0, 128, 128, 255};
        setFallbackColor(pixel);
    }
    shader.setMatrix4("model", transform, 1, GL_FALSE);
    shader.setTexture("texture", GL_TEXTURE_2D, 0, meshTexture.texture.TCB ? meshTexture.texture.TCB : texture_fallback);
    
    GLVertexElement<Vertex2D>::draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Mesh2D::init(CacheApproach::VRAM_Approach approach) {
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