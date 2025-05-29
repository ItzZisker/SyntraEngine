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
    // model data
    std::vector<Texture> textures_loaded; // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool loaded, gammaCorrection;
    
    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path, bool gamma = false);

    // destructor
    ~Model();

    // Renders Model using BatchShader
    void render(GameWindow *window) override;

    // draws the model, and thus all its meshes
    void draw(Shader &shader);

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(bool flipTextures = false);

private:
    std::string path;

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};