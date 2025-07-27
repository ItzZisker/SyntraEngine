#pragma once

#include "engine/RenderTable.hpp"
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
#include <world/WorldObject.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace syng
{
unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model
{
public:
    std::set<std::string> renderable_meshes;
    std::vector<Texture> textures_loaded;
    std::map<std::string, Mesh*> meshes;
    std::string directory;
    bool loaded, gammaCorrection;
    
    Model(std::string const &path, bool gamma = false);

    ~Model();

    void filterMesh(std::string meshName);

    void draw(Shader &shader);

    void read(const std::set<std::string>& meshes = {}, bool flipTextures = false);

    void readAll(bool flipTextures = false) {
        read({}, flipTextures);
    }

    void load(VRAM_Approach approach = Sequential);

    void loadModel(VRAM_Approach approach = Sequential, const std::set<std::string>& meshes = {}, bool flip = false);
private:
    std::string path;

    void processNode(const std::set<std::string>& meshNames, aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform);

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4& transform);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
}