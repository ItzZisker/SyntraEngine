#pragma once

#include "Syngine/modules/Screenbuffer.hpp"
#include "glm/fwd.hpp"
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

struct MaterialProps {
    glm::vec3 ior = glm::vec3(1.0f);
    float shininess = 32.0f;
    float minOpacity = 0.7f, maxOpacity = 1.0f;
    float opacity = 1.0f;
    float F0 = 0.04f;
    bool isTransparent = false;
};

class Mesh {
private:
    glm::mat4 parentToNodeTransform;
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    MaterialProps material;

    unsigned int VAO;
    unsigned int VBO, EBO;

    bool loaded;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, glm::mat4 parentToNodeTransform);

    ~Mesh();

    glm::mat4 getParentToNodeTransform();

    void render(Shader shader, Screenbuffer screen, glm::mat4 transform);

    void init(VRAM_Approach = Sequential);
};