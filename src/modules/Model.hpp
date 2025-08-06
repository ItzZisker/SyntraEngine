#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Syngine.hpp>
#include <modules/Mesh.hpp>
#include <modules/Shader.hpp>
#include <unordered_map>
#include <world/WorldObject.hpp>

#include <string>
#include <vector>

namespace syng
{
unsigned int TCBFromFile(const char *path, const std::string &directory);

Texture TextureFromFile(const char *path, const std::string &directory, const std::string &type);

class Model
{
public:
    std::set<std::string> renderable_meshes;
    std::vector<Texture> textures_loaded;
    std::unordered_map<std::string, Mesh*> meshes;
    std::unordered_map<std::string, std::unordered_map<std::string, Mesh*>> meshGroups;
    std::string directory;
    bool loaded, gammaCorrection;
    
    Model(std::string const &path, bool gamma = false);

    ~Model();

    void filterMesh(std::string meshName);

    void draw(Shader &shader);

    void read(
        const std::set<std::string>& meshes = {},
        bool flipTextures = false,
        aiPostProcessSteps postProcessSteps = static_cast<aiPostProcessSteps>(
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
        )
    );

    void load(VRAM_Approach approach = Sequential);

    void loadModel(VRAM_Approach approach = Sequential, const std::set<std::string>& meshes = {}, bool flip = false);

    void groupMeshes();

    void pushTexture(const std::string meshKey, Texture texture);
private:
    std::string path;

    void processNode(const std::set<std::string>& meshNames, aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform);

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4& transform);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
}