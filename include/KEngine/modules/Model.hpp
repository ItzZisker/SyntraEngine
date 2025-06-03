#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <KEngine/KEngine.hpp>
#include <KEngine/modules/Mesh.hpp>
#include <KEngine/modules/Shader.hpp>
#include <KEngine/world/WorldObject.hpp>

#include <string>
#include <vector>

unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model : public CoordinatedObject
{
public:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh*> meshes;
    std::string directory;
    bool loaded, gammaCorrection;
    
    Model(std::string const &path, bool gamma = false);

    ~Model();

    void render(GameWindow *window) override;

    void draw(Shader &shader);

    void loadModel(bool flipTextures = false);

private:
    std::string path;

    void processNode(aiNode *node, const aiScene *scene);

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};