#pragma once

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include "Syngine/serialization/DataSerializer.hpp"

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <filesystem>
#include <string>
#include <vector>
#include <utility>

namespace syng
{
constexpr uint16_t PCK_HEADER_MODEL = 100;
constexpr uint16_t PCK_FOOTER_MODEL = 101;

using TexelPair = std::pair<MaterialTexture2D_T, Texture2D>;
using TexelPairs = std::vector<TexelPair>;

class Model;

struct NamedMesh {
    std::string name = "NONE";
    Mesh* mesh = nullptr;
};

class LocalNode {
public:
    std::string name = "NONE";
    glm::mat4 transform = glm::mat4(1.0f);
    
    std::vector<LocalNode> children = {};
    std::vector<NamedMesh> meshes = {};

    LocalNode(std::string name, glm::mat4 transform = glm::mat4(1.0f));

    void purgeMeshes();

    bool hasMesh();
    bool isEmpty();
};

class PackedWriter {
private:
    DataSerializer* buffer;
public:
    PackedWriter(DataSerializer* buffer);
    void write(Model* model);
};

class PackedReader {
private:
    DataDeserializer* buffer;
public:
    bool flipTextures = false;

    PackedReader(DataDeserializer* buffer);
    void read(Model* model);
};

#ifdef USE_ASSIMP
class AssimpReader {
private:
    std::filesystem::path path;
    TexelPairs cachedTextures;

    void processNode(Model *model, aiNode *node, const aiScene *scene, LocalNode& wmt);
    Mesh* processMesh(Model *model, aiMesh *mesh, const aiScene *scene);
    void cacheMaterialTextures(
        aiMaterial *mat, Material *syngMat,
        aiTextureType type, const MaterialTexture2D_T &syngType
    );
public:
    aiPostProcessSteps postProcessSteps = static_cast<aiPostProcessSteps>(
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace
    );
    bool flipTextures = false;

    AssimpReader(const std::filesystem::path& path);

    TexelPairs& getCachedTextures();
    void read(Model* model);
};
#endif

class Model {
protected:
    bool uploaded = false;
public:
    LocalNode rootNode = {"ROOT"};
    std::vector<Material*> materialById;

    Model();
    ~Model();

    void draw(Shader &shader);

    void serialize(PackedWriter writer);
    void readPacked(PackedReader reader);
#ifdef USE_ASSIMP
    void readAssimp(AssimpReader reader);
#endif

    bool isUploaded() { return this->uploaded; };
    void uploadVertices(CacheApproach::VRAM_Approach approach = CacheApproach::Sequential);
};
}