#include "Model.hpp"

#include "Mesh.hpp"
#include "Texture.hpp"

#include "serialization/DataSerializer.hpp"
#include "serialization/DataTemplates.hpp"

#include "utils/GameUtils.hpp"
#include <filesystem>

#ifdef USE_ASSIMP
#include "assimp/matrix4x4.h"
#endif

#include <unordered_map>
#include <ostream>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <regex>

using namespace syng;

Model::Model() {}

Model::~Model() {
    meshGroups.clear();
    for (auto& mesh : meshes) delete mesh.second;
    meshes.clear();
    loaded = false;
}

void Model::load(CacheApproach::VRAM_Approach approach) {
    if (meshes.empty()) return;
    for (const auto& pair : meshes) { pair.second->init(approach); }
    groupMeshes();
    loaded = true;
}

void Model::groupMeshes() {
    meshGroups.clear();
    std::regex baseNameRegex(R"(^(.*)-\d+$)");

    for (const auto& [name, meshPtr] : meshes) {
        std::smatch match;
        std::string baseName = name;

        if (std::regex_match(name, match, baseNameRegex) && match.size() == 2) {
            baseName = match[1]; // Extract "Cube.042" from "Cube.042-0"
        }
        meshGroups[baseName][name] = meshPtr;
    }
}

void Model::serialize(PackedWriter writer) {
    writer.write(this);
}

void Model::readPacked(PackedReader reader) {
    reader.read(this);
}

#ifdef USE_ASSIMP
void Model::readAssimp(AssimpReader reader) {
    reader.read(this);
}
#endif

void Model::pushTexture(const std::string& meshKey, MeshTexture2D texture) {
    auto pair = meshes.find(meshKey);
    if (pair == meshes.end()) return;
    Mesh *mesh = pair->second;
    mesh->textures.push_back(texture);

    if (texture.type == Texture_Height) {
        mesh->material.hasDisplacement = true;
    }
    if (texture.type == Texture_Rough) {
        mesh->material.hasRoughness = true;
    }
    
}

void Model::pullTexture(const std::string& meshKey, const std::string& path) {
    auto meshIt = meshes.find(meshKey);
    if (meshIt == meshes.end()) return;
    Mesh* mesh = meshIt->second;
    mesh->textures.erase(
        std::remove_if(mesh->textures.begin(), mesh->textures.end(),
        [&](const MeshTexture2D& tex) {
            if (tex.texture.path == path) {
                if (tex.type == Texture_Height) {
                    mesh->material.hasDisplacement = false;
                }
                if (tex.type == Texture_Rough) {
                    mesh->material.hasRoughness = false;
                }
                return true;
            }
            return false;
        }),
        mesh->textures.end()
    );
}

#ifdef USE_ASSIMP
AssimpReader::AssimpReader(const std::filesystem::path& path) : path(path) {}

std::vector<MeshTexture2D>& AssimpReader::getCachedTextures() {
    return this->cachedTextures;
}

void AssimpReader::read(Model* model) {
    if (model->isLoaded() || !model->meshes.empty()) return;

    stbi_set_flip_vertically_on_load(flipTextures);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path.string(), postProcessSteps);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    MeshKeyedMap import = processNode(scene->mRootNode, scene, aiMatrix4x4());
    model->meshes.insert(import.begin(), import.end());
}

MeshKeyedMap AssimpReader::processNode(aiNode *node, const aiScene *scene, const aiMatrix4x4& parentTransform) {
    MeshKeyedMap nodeMeshes;
    aiMatrix4x4 currentTransform = parentTransform * node->mTransformation;

    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        glm::mat4 glmTransform = GameUtils::convertToGLMMatrix(currentTransform);

        nodeMeshes.insert({mesh->mName.C_Str(), processMesh(mesh, scene, glmTransform)});
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        MeshKeyedMap sub = processNode(node->mChildren[i], scene, currentTransform);
        nodeMeshes.insert(sub.begin(), sub.end());
    }
    return nodeMeshes;
}

Mesh* AssimpReader::processMesh(aiMesh *mesh, const aiScene *scene, const glm::mat4& transform) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<MeshTexture2D> textures;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector(0.0f);

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec(0.0f);

            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<MeshTexture2D> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture_Diffuse);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<MeshTexture2D> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture_Specular);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<MeshTexture2D> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, Texture_Normal);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<MeshTexture2D> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, Texture_Height);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    std::vector<MeshTexture2D> roughMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, Texture_Rough);
    textures.insert(textures.end(), roughMaps.begin(), roughMaps.end());

    MaterialProps props;

    float ior = 1.0f;
    float opacity = 1.0f;

    if (material->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS) {
        props.ior = glm::vec3(ior);
    }
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        props.opacity = opacity;
        props.isTransparent = (opacity < 1.0f);
    }
    props.hasDisplacement = !heightMaps.empty();
    props.hasRoughness = !roughMaps.empty();

    Mesh* res = new Mesh(vertices, indices, transform);
    res->textures = textures;
    res->material = props;

    return res;
}

std::vector<MeshTexture2D> AssimpReader::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const MeshTexture2D_T &texType) {
    std::vector<MeshTexture2D> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString texName;
        mat->GetTexture(type, i, &texName);
        bool skip = false;
        for (unsigned int j = 0; j < cachedTextures.size(); j++) {
            if (std::strcmp(cachedTextures[j].texture.path.data(), texName.C_Str()) == 0) {
                textures.push_back(cachedTextures[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            MeshTexture2D texture = loadMeshTexture2D(path.parent_path() / texName.C_Str(), texType);
            textures.push_back(texture);
            cachedTextures.push_back(texture);
        }
    }
    return textures;
}
#endif

PackedReader::PackedReader(DataDeserializer* buff) : buffer(buff) {}

void PackedReader::read(Model* model) {
    DataTemplates::push(buffer, "Model", PCK_HEADER_MODEL);

    std::vector<MeshTexture2D> textures_all = DataTemplates::read_vector<MeshTexture2D>(buffer, [&](){
        return loadMeshTexture2D(buffer);
    });

    uint32_t map_size = DataTemplates::read_uint32(buffer);
    for (uint32_t i = 0; i < map_size; i++) {
        std::string meshKey = DataTemplates::read_string(buffer);
        glm::mat4 parentToNodeTransform = DataTemplates::read_glm_mat4(buffer);

        std::vector<Vertex> vertices = DataTemplates::read_vector<Vertex>(buffer, [&]() {
            Vertex result;
            result.position = DataTemplates::read_glm_vec3(buffer);
            result.normal = DataTemplates::read_glm_vec3(buffer);
            result.texCoords = DataTemplates::read_glm_vec2(buffer);
            result.tangent = DataTemplates::read_glm_vec3(buffer);
            result.bitangent = DataTemplates::read_glm_vec3(buffer);
            for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
                result.m_BoneIDs[i] = DataTemplates::read_int32(buffer);
                result.m_Weights[i] = DataTemplates::read_float(buffer);
            }
            return result;
        });
        std::vector<uint32_t> indices = DataTemplates::read_vector<uint32_t>(buffer, [&](){
            return DataTemplates::read_uint32(buffer);
        });
        std::vector<MeshTexture2D> subtextures = DataTemplates::read_vector<MeshTexture2D>(buffer, [&](){
            return textures_all[DataTemplates::read_uint32(buffer)];
        });

        MaterialProps material = DataTemplates::read_mesh_material(buffer);

        Mesh* mesh = new Mesh(vertices, indices, parentToNodeTransform);
        mesh->textures = subtextures;
        mesh->material = material;

        model->meshes.insert({meshKey, mesh});
    }
    DataTemplates::pop(buffer, "Model", PCK_FOOTER_MODEL);
}

PackedWriter::PackedWriter(DataSerializer* buffer) : buffer(buffer) {}

void PackedWriter::write(Model* model) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_MODEL);

    std::vector<MeshTexture2D> textures_all;
    std::unordered_map<std::string, uint32_t> textures_all_indices;
    std::unordered_map<std::string, std::vector<uint8_t>> textures_all_data;

    uint32_t texelIndex = 0;
    for (auto& pair : model->meshes) {
        Mesh* mesh = pair.second;
        for (auto& mT : mesh->textures) {
            if (textures_all_indices.find(mT.texture.path) == textures_all_indices.end()) {
                std::ifstream textureFile;

                textureFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                textureFile.open(mT.texture.path, std::ios::binary);
                std::vector<uint8_t> texel_bytes((std::istreambuf_iterator<char>(textureFile)), {});
                textureFile.close();

                textures_all.push_back(mT);
                textures_all_indices[mT.texture.path] = texelIndex++;
                textures_all_data[mT.texture.path] = std::move(texel_bytes);
            }
        }
    }

    DataTemplates::write_vector<MeshTexture2D>(buffer, textures_all, [&](const MeshTexture2D& mT){
        TextureWriter(buffer).writeMeshTexture2D(mT.texture.path, textures_all_data[mT.texture.path], mT.type);
    });

    DataTemplates::write_uint32(buffer, model->meshes.size());
    for (auto& pair : model->meshes) {
        std::string meshKey = pair.first;
    
        Mesh* mesh = pair.second;
        std::vector<Vertex> vertices = mesh->getVertices();
        std::vector<GLuint> indices = mesh->getIndices();

        DataTemplates::write_string(buffer, meshKey);
        DataTemplates::write_glm_mat4(buffer, mesh->getParentToNodeTransform());
        DataTemplates::write_vector<Vertex>(buffer, vertices, [&](const Vertex& v){
            DataTemplates::write_glm_vec3(buffer, v.position);
            DataTemplates::write_glm_vec3(buffer, v.normal);
            DataTemplates::write_glm_vec2(buffer, v.texCoords);
            DataTemplates::write_glm_vec3(buffer, v.tangent);
            DataTemplates::write_glm_vec3(buffer, v.bitangent);
            for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++) { 
                DataTemplates::write_int32(buffer, v.m_BoneIDs[i]);
                DataTemplates::write_float(buffer, v.m_Weights[i]);
            }
        });
        DataTemplates::write_vector<GLuint>(buffer, indices, [&](const GLuint& index){
            LittleEndian::write<GLuint>(buffer, index);
        });
        DataTemplates::write_vector<MeshTexture2D>(buffer, mesh->textures, [&](const MeshTexture2D& subtexture){
            LittleEndian::write<uint32_t>(buffer, textures_all_indices[subtexture.texture.path]);
        });
        DataTemplates::write_mesh_material(buffer, mesh->material);
    }

    DataTemplates::write_uint16(buffer, PCK_FOOTER_MODEL);
}