#include "engine/Config.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "glm/fwd.hpp"
#include <modules/Mesh.hpp>
#include <utils/GameUtils.hpp>
#include <cmath>

using namespace syng;

GLuint defaultWhiteTexture;

void createDefaultWhiteTexture() {
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };

    glGenTextures(1, &defaultWhiteTexture);
    glBindTexture(GL_TEXTURE_2D, defaultWhiteTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint syng::getDefaultWhiteTexture() {
    if (!defaultWhiteTexture) createDefaultWhiteTexture();
    return defaultWhiteTexture;
}

Mesh::Mesh(std::vector<Vertex> vertices,
           std::vector<unsigned int> indices,
           glm::mat4 parenToNodeTransform)
    : vertices(vertices), indices(indices), parentToNodeTransform(parenToNodeTransform) {
}

Mesh::~Mesh() {
    vertices.clear();
    indices.clear();
    textures.clear();

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

glm::mat4 Mesh::getParentToNodeTransform() {
    return parentToNodeTransform;
}

void Mesh::render(Shader shader, Screenbuffer screen, glm::mat4 transform) {
    if (!loaded) return;
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());

    shader.use();
    shader.setMatrix4("model", transform, 1, GL_FALSE);
    shader.setFloat("F0", material.F0);
    shader.setVec3f("ior", material.ior);
    shader.setFloat("shininess", material.shininess);

    if (shader.getVariable(SHADER_REFRAC_KEY_DYNAMIC_OPACITY) == SHADER_VAL_ON) {
        shader.setFloat("minOpacity", material.minOpacity);
        shader.setFloat("maxOpacity", material.maxOpacity);
    } else {
        shader.setFloat("opacity", material.opacity);
    }

    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    if (textures.empty()) {
        shader.setTexture("texture_diffuse", GL_TEXTURE_2D, 0, getDefaultWhiteTexture());
    }

    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string number;
        std::string name = textures[i].type;
        
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        shader.setTexture(name + number, GL_TEXTURE_2D, i, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Mesh::init(VRAM_Approach approach) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    switch (approach) {
        case Sequential:
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoords));

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tangent));

            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitangent));

            glEnableVertexAttribArray(5);
            glVertexAttribIPointer(5, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
            glBindVertexArray(0);
        break;
        case Interleaved:
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
        
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
        
            size_t totalSize =
                vec3fLength * sizeof(float) * 3 +
                vec2fLength * sizeof(float) +
                boneLength * sizeof(int) +
                boneLength * sizeof(float);

            glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

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

            glBufferSubData(GL_ARRAY_BUFFER, offset, vec3fLength * sizeof(float), positions);
            offset += vec3fLength * sizeof(float);

            glBufferSubData(GL_ARRAY_BUFFER, offset, vec3fLength * sizeof(float), normals);
            offset += vec3fLength * sizeof(float);

            glBufferSubData(GL_ARRAY_BUFFER, offset, vec2fLength * sizeof(float), texCoords);
            offset += vec2fLength * sizeof(float);
            
            glBufferSubData(GL_ARRAY_BUFFER, offset, vec3fLength * sizeof(float), tangents);
            offset += vec3fLength * sizeof(float);

            glBufferSubData(GL_ARRAY_BUFFER, offset, vec3fLength * sizeof(float), biTangents);
            offset += vec3fLength * sizeof(float);

            glBufferSubData(GL_ARRAY_BUFFER, offset, boneLength * sizeof(int), m_BoneIDs);
            offset += boneLength * sizeof(int);

            glBufferSubData(GL_ARRAY_BUFFER, offset, boneLength * sizeof(float), m_Weights);

            GLuint attribIndex = 0;
            offset = 0;

            glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += vec3fLength * sizeof(float);
           
            glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += vec3fLength * sizeof(float);
           
            glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += vec2fLength * sizeof(float);

            glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += vec3fLength * sizeof(float);

            glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += vec3fLength * sizeof(float);

            glVertexAttribIPointer(attribIndex, MAX_BONE_INFLUENCE, GL_INT, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            offset += boneLength * sizeof(int);

            glVertexAttribPointer(attribIndex, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, 0, (void*)(offset));
            glEnableVertexAttribArray(attribIndex++);
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            delete[] positions;
            delete[] normals;
            delete[] texCoords;
            delete[] tangents;
            delete[] biTangents;
            delete[] m_Weights;
            delete[] m_BoneIDs;
        break;
    }

    loaded = true;
}