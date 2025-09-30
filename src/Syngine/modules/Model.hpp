#pragma once

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include "Syngine/serialization/DataSerializer.hpp"

#include <glad/glad.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

namespace syng
{
constexpr uint16_t PCK_HEADER_MODEL = 100;
constexpr uint16_t PCK_FOOTER_MODEL = 101;

using MeshKeyedMap = std::unordered_map<std::string, Mesh*>;

using TexelPair = std::pair<MaterialTexture2D_T, Texture2D>;
using TexelPairs = std::vector<TexelPair>;

class Model;

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

    void processNode(Model *model, aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform);
    Mesh* processMesh(Model *model, aiMesh *mesh, const aiScene *scene, const glm::mat4& transform);
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

class Model
{
private:
    bool uploaded = false;
public:
    MeshKeyedMap meshes;
    std::unordered_map<std::string, MeshKeyedMap> meshGroups;
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
    void groupMeshes();
};
}