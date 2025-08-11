#pragma once

#include "modules/GLObjects.hpp"
#include "modules/Screenbuffer.hpp"
#include "glm/fwd.hpp"
#include "world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <modules/Shader.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#define MAX_BONE_INFLUENCE 4

namespace syng
{
enum VRAM_Approach {
    Sequential,
    Interleaved
};

enum Texture_T {
    Texture_Diffuse,
    Texture_Specular,
    Texture_Normal,
    Texture_Height,
    Texture_Rough
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

struct Vertex2D {
    glm::vec2 position;
    glm::vec2 texCoords;
};

struct Texture {
    unsigned int TCB;
    Texture_T type;
    std::string path;
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

constexpr std::array<const char*, 5> TextureTNames = {
    "texture_diffuse",
    "texture_specular",
    "texture_normal",
    "texture_height",
    "texture_roughness"
};
constexpr const char* TEXTURE_NAME(Texture_T type) {
    auto i = static_cast<size_t>(type);
    if (i >= TextureTNames.size()) {
        return "Unknown";
    }
    return TextureTNames[i];
}

Texture loadTexture(const std::string& path, const std::string& type);

class Mesh : public GLVertexElement<Vertex> {
private:
    glm::mat4 parentToNodeTransform;
    std::unordered_map<Texture_T, GLuint> textures_fallback;
    std::vector<Texture> textures;
public:
    MaterialProps material;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, glm::mat4 parentToNodeTransform);
    ~Mesh();

    glm::mat4 getParentToNodeTransform();
    Coordination getParentToNodeCoords();

    bool hasFallback(Texture_T texType);
    void setFallbackTCB(Texture_T texType, GLuint TCB);
    void setFallbackColor(Texture_T texType, GLubyte pixel[4]);

    void render(Shader shader, Screenbuffer screen, glm::mat4 transform);
    void init(VRAM_Approach = Sequential);

    std::vector<Texture>& getTextures() { return this->textures; };
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
    Texture texture = {0};
public:
    Mesh2D(std::vector<Vertex2D> vertices, std::vector<GLuint> indices, glm::mat4 parentToNodeTransform);
    ~Mesh2D();

    glm::mat4 getParentToNodeTransform();
    Coordination getParentToNodeCoords();

    void setFallbackTCB(GLuint TCB);
    void setFallbackColor(GLubyte pixel[4]);
    void setTexture(Texture texture);

    void render(Shader shader, Screenbuffer screen, glm::mat4 transform);
    void init(VRAM_Approach = Sequential);

    Texture& getTexture() { return this->texture; }
    GLuint getFallbackTCB() { return this->texture_fallback; }
};
}