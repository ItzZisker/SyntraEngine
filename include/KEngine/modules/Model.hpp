#pragma once

#include "KEngine/engine/RenderTable.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <KEngine/KEngine.hpp>
#include <KEngine/modules/Mesh.hpp>
#include <KEngine/modules/Shader.hpp>
#include <KEngine/world/WorldObject.hpp>

#include <string>
#include <unordered_map>
#include <vector>

unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model : public ShaderRenderable
{
public:
    std::set<std::string> renderable_meshes;
    std::vector<Texture> textures_loaded;
    std::unordered_map<std::string, Mesh*> meshes;
    std::string directory;
    bool loaded, gammaCorrection;
    
    Model(std::string const &path, bool gamma = false);

    ~Model();

    void render(Shader shader, int FBO) override;

    void draw(Shader &shader);

    void loadModel(const std::set<std::string>& meshes, bool flipTextures = false);

    void loadModel(bool flipTextures = false);
private:
    std::string path;

    void processNode(const std::set<std::string>& meshNames, aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform);

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4& transform);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};