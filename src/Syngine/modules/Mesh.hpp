#pragma once

#include "GLObjects.hpp"
#include "Texture.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace syng
{

namespace CacheApproach
{
enum VRAM_Approach {
    Sequential,
    Interleaved
};
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Vertex2D {
    glm::vec2 position;
    glm::vec2 texCoords;
};

struct MaterialProps {
    glm::vec3 ior = glm::vec3(1.0f);
    float shininess = 32.0f;
    float minOpacity = 0.7f, maxOpacity = 1.0f; // Used if dynamic opacity is enabled within shader
    float opacity = 1.0f;
    float F0 = 0.04f;
    bool isTransparent = false;
    bool hasDisplacement = true;
    bool hasRoughness = true;
};

class Mesh : public GLVertexElement<Vertex> {
private:
    glm::mat4 parentToNodeTransform;
    std::unordered_map<MeshTexture2D_T, GLuint> textures_fallback;
public:
    std::vector<MeshTexture2D> textures;
    MaterialProps material;

    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::mat4 parentToNodeTransform);
    ~Mesh();

    glm::mat4 getParentToNodeTransform();
    Coordination getParentToNodeCoords();

    bool hasFallback(MeshTexture2D_T texType);
    void setFallbackTCB(MeshTexture2D_T texType, GLuint TCB);
    void setFallbackColor(MeshTexture2D_T texType, GLubyte pixel[4]);

    void render(Shader& shader, Screenbuffer screen, glm::mat4 transform);
    void init(CacheApproach::VRAM_Approach = CacheApproach::Sequential);
};

// TODO: This
// - Replace all messy vertex arrays in all modules with new GLVertex and GLVertexElement
// - Text is also an implementation of Mesh2D which could have font, animations, etc
// - Mesh2D could be sprite, quads, menu buttons, animated, or any 2D object
// - Unfolded Spherical One-Pass Shadow Maps
class Mesh2D : public GLVertexElement<Vertex2D> {
private:
    glm::mat4 parentToNodeTransform = glm::mat4(1.0f);
    GLuint texture_fallback = 0;
    MeshTexture2D meshTexture = {0};
public:
    Mesh2D(std::vector<Vertex2D> vertices, std::vector<GLuint> indices, glm::mat4 parentToNodeTransform);
    ~Mesh2D();

    glm::mat4 getParentToNodeTransform();
    Coordination getParentToNodeCoords();

    void setFallbackTCB(GLuint TCB);
    void setFallbackColor(GLubyte pixel[4]);
    void setTexture(MeshTexture2D texture);

    void render(Shader& shader, Screenbuffer screen, glm::mat4 transform);
    void init(CacheApproach::VRAM_Approach = CacheApproach::Sequential);

    MeshTexture2D& getTexture() { return this->meshTexture; }
    GLuint getFallbackTCB() { return this->texture_fallback; }
};
}