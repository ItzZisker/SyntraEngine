#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Syngine/modules/Shader.hpp>

#include <string>
#include <vector>

#define MAX_BONE_INFLUENCE 4

GLuint getDefaultWhiteTexture();

enum VRAM_Approach {
    Sequential,
    Interleaved
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh : public CoordinatedObject, public ShaderRenderable {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    unsigned int VAO;
    unsigned int VBO, EBO;

    bool loaded;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    ~Mesh();

    void render(Shader shader, int FBO) override;

    void init(VRAM_Approach = Sequential);
};