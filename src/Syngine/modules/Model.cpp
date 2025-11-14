#include "Model.hpp"

#include "Mesh.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Texture.hpp"

#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/serialization/DataTemplates.hpp"
#include "Syngine/utils/GameUtils.hpp"

#ifdef USE_ASSIMP
#include "assimp/vector3.h"
#include "assimp/material.h"
#endif

#include "stb_image.h"

#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <string>
#include <vector>

using namespace syng;

NamedMesh::~NamedMesh() { delete mesh; }

LocalNode::LocalNode(std::string name, glm::mat4 transform) : name(name), transform(transform) {}

bool LocalNode::hasMesh() {
    return !meshes.empty();
}

bool LocalNode::isEmpty() {
    return meshes.empty() && children.empty();
}

Model::Model() {}

Model::~Model() {
    rootNode.meshes.clear();
    rootNode.children.clear();
    for (auto *nmesh : meshesById) delete nmesh;
    meshesById.clear();
    for (Material *mat : materialById) delete mat;
    materialById.clear();
    uploaded = false;
}

void Model::uploadVertices(CacheApproach::VRAM_Approach approach, bool uploadPBR) {
    for (auto *nmesh : meshesById) nmesh->mesh->uploadVertices(approach, uploadPBR);
    uploaded = true;
}

void Model::uploadTextures(std::function<void(GLuint TCB)> params, bool remove_from_mem) {
    for (auto *mat : materialById) {
        for (auto &list : mat->textures) {
            for (auto &texture : list.second) {
                texture.uploadTexture(params);
                if (remove_from_mem) texture.clearImage();
            }
        }
    }
}

void Model::serialize(ModelIO::PackedWriter writer) {
    writer.write(this);
}

void Model::readPacked(ModelIO::PackedReader reader) {
    reader.read(this);
}

#ifdef USE_ASSIMP
void Model::readAssimp(ModelIO::AssimpReader reader) {
    reader.read(this);
}
#endif

#ifdef USE_ASSIMP
ModelIO::AssimpReader::AssimpReader(const std::filesystem::path& path) : path(path) {}

TexelPairs& ModelIO::AssimpReader::getCachedTextures() {
    return this->cachedTextures;
}

void ModelIO::AssimpReader::read(Model* model) {
    if (model->isUploaded() || !model->rootNode.isEmpty()) return;

    stbi_set_flip_vertically_on_load(flipTextures);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path.string(), postProcessSteps);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    processNode(model, scene->mRootNode, scene, model->rootNode);
}

void ModelIO::AssimpReader::processNode(Model* model, aiNode* node, const aiScene* scene, LocalNode& gNode) {
    gNode.name = std::string(node->mName.C_Str());
    gNode.transform = glm::mat4(GameUtils::convertToGLMMatrix(node->mTransformation));

    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        int meshID = node->mMeshes[i];
        aiMesh* mesh = scene->mMeshes[meshID];
        std::string meshName = std::string(mesh->mName.C_Str());
        if (meshName.empty()) meshName = "Unnamed";
        if (meshID >= model->meshesById.size()) {
            model->meshesById.resize(meshID + 1);
        }
        if (!model->meshesById[meshID]) {
            NamedMesh *nmesh = new NamedMesh();
            nmesh->name = meshName;
            nmesh->mesh = processMesh(meshID, model, mesh, scene);
            model->meshesById[meshID] = nmesh;
        }
        gNode.meshes.push_back(model->meshesById[meshID]);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        LocalNode child("Unnamed");
        processNode(model, node->mChildren[i], scene, child);
        gNode.children.push_back(child);
    }
}


Mesh* ModelIO::AssimpReader::processMesh(int meshID, Model *model, aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    TexelPairs textures;

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
            vertex.texCoords0 = vec;

            aiVector3D T = mesh->mTangents[i], B = mesh->mBitangents[i];

            glm::vec3 Tglm = glm::vec3(T.x, T.y, T.z);
            glm::vec3 Bglm = glm::vec3(B.x, B.y, B.z);

            vertex.tangent = glm::vec3(Tglm) * glm::dot(glm::cross(vertex.normal, Tglm), Bglm);
        } else {
            vertex.texCoords0 = glm::vec2(0.0f, 0.0f);
        }
        if (mesh->mTextureCoords[1]) { // Used internally for PBR: AO, LightMaps, detail textures, etc.
            glm::vec2 vec(0.0f);
            vec.x = mesh->mTextureCoords[1][i].x;
            vec.y = mesh->mTextureCoords[1][i].y;
            vertex.texCoords1 = vec;
        } else {
            vertex.texCoords1 = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    if (mesh->mMaterialIndex >= model->materialById.size()) {
        model->materialById.resize(mesh->mMaterialIndex + 1);
    }

    if (!model->materialById[mesh->mMaterialIndex]) {
        MetaDataMap metadata_map;
        MaterialProps props;

        for (int i = 0; i < material->mNumProperties; i++) {
            auto property = material->mProperties[i];
            metadata_map.emplace(
                property->mKey.C_Str(),
                M_Metadata(
                    property->mType,
                    property->mData,
                    property->mDataLength
                )
            );
        }

        props.diffuseColor = getMColor(material, AI_MATKEY_COLOR_DIFFUSE);
        props.specularColor = getMColor(material, AI_MATKEY_COLOR_SPECULAR);
        props.baseColor = getMColor(material, AI_MATKEY_BASE_COLOR);

        float alphaCutoff = 0.0f;
        if (loadPBRTextures) {
            props.pbr.emissiveColor = getMColor(material, AI_MATKEY_COLOR_EMISSIVE);
            if (material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == AI_SUCCESS) {
                props.pbr.emissiveColor.w = alphaCutoff;
            } else {
                props.pbr.emissiveColor.w = 0.5f;                
            }
        }

        float ior = 1.0f;
        float opacity = 1.0f;
        if (material->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS) {
            props.ior = glm::vec3(ior);
        }
        if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
            props.opacity = opacity;
            props.maxOpacity = opacity;
            props.isTransparent = (opacity < 1.0f);
            if (loadPBRTextures) {
                static float opaquenessThreshold = 0.05f;
                props.pbr.transparencyFactor = std::clamp(1.0f - opacity, 0.0f, 1.0f);
                if (props.pbr.transparencyFactor >= 1.0f - opaquenessThreshold) {
                    props.pbr.transparencyFactor = 0.0f;
                }
            }
        }
        if (loadPBRTextures) {
            float MetallicFactor;
            if (material->Get(AI_MATKEY_METALLIC_FACTOR, MetallicFactor) == AI_SUCCESS) {
                props.pbr.metallicRoughnessNormalOcclusion.x = MetallicFactor;
            }
            float RoughnessFactor;
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, RoughnessFactor) == AI_SUCCESS) {
                props.pbr.metallicRoughnessNormalOcclusion.y = RoughnessFactor;
            }
            float NormalScale;
            if (material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), NormalScale) == AI_SUCCESS) {
                props.pbr.metallicRoughnessNormalOcclusion.z = NormalScale;
            }
            float OcclusionStrength;
            if (material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_LIGHTMAP, 0), OcclusionStrength) == AI_SUCCESS) {
                props.pbr.metallicRoughnessNormalOcclusion.w = OcclusionStrength;
            }
        }
        material->Get(AI_MATKEY_SHININESS, props.shininess);

        Material *syngMat = new Material(mesh->mMaterialIndex, std::string(material->GetName().C_Str()), props, metadata_map, loadPBRTextures);
        model->materialById[mesh->mMaterialIndex] = syngMat;
    
        static std::vector<MaterialTexture2D_T> blinnPhongtypes = {
            Texture_Diffuse,
            Texture_Specular,
            Texture_Normal,
            Texture_Height,
            Texture_Rough
        };
        static std::vector<MaterialTexture2D_T> pbrtypes = {
            Texture_Diffuse,
            Texture_Specular,
            Texture_Normal,
            Texture_Height,
            Texture_Rough,
            Texture_Metallic,
            Texture_AmbientOcclusion,
            Texture_Emissive
        };
        if (loadPBRTextures) {
            for (auto& type: pbrtypes)
                cacheMaterialTextures(material, syngMat, TEXTURE_ASSIMP(type), type);
        } else {
            for (auto& type: blinnPhongtypes)
                cacheMaterialTextures(material, syngMat, TEXTURE_ASSIMP(type), type);
        }
    }

    Mesh* result = new Mesh(meshID, vertices, indices);
    result->setMaterial(model->materialById[mesh->mMaterialIndex]);

    return result;
}

glm::vec4 ModelIO::AssimpReader::getMColor(
    aiMaterial* pMaterial, const char* pAiMatKey,
    int AiMatType, int AiMatIdx
) {
    glm::vec4 color;
    aiColor4D AiColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(pAiMatKey, AiMatType, AiMatIdx, AiColor) == AI_SUCCESS) {
        color.x = AiColor.r;
        color.y = AiColor.g;
        color.z = AiColor.b;
        color.w = std::min(AiColor.a, 1.0f);
    } else {
        color = glm::vec4(1.0f);
    }
    return color;
}

void ModelIO::AssimpReader::cacheMaterialTextures(
    aiMaterial *mat, Material *syngMat,
    aiTextureType type, const MaterialTexture2D_T &syngType
) {
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString texName;
        mat->GetTexture(type, i, &texName);
        std::string texNameC = std::string(texName.C_Str());
        GameUtils::str_replaceAll(texNameC, "%20", " ");

        if (syngType == Texture_Rough) syngMat->props.hasRoughness = true;
        if (syngType == Texture_Height) syngMat->props.hasDisplacement = true;

        bool skip = false;
        for (unsigned int j = 0; j < cachedTextures.size(); j++) {
            if (std::strcmp(cachedTextures[j].second.getPath().data(), texNameC.c_str()) == 0) {
                skip = true;
                break;
            }
        }
        if (!skip) {
            Texture2D texture = TextureIO::TextureFileReader::readTexture2D(path.parent_path() / texNameC.c_str());
            cachedTextures.push_back({syngType, texture});
            syngMat->addTexture(syngType, texture);
        }
    }
}
#endif

ModelIO::PackedReader::PackedReader(DataDeserializer* buff) : buffer(buff) {}

void ModelIO_PackedReader_ReadNode(Model *model, DataDeserializer *buffer, LocalNode& node) {
    node.name = DataTemplates::read_string(buffer);
    node.transform = DataTemplates::read_glm_mat4(buffer);
    node.meshes = DataTemplates::read_vector<NamedMesh*>(buffer, [&](){
        return model->meshesById[DataTemplates::read_varint(buffer)];
    });
    node.children = DataTemplates::read_vector<LocalNode>(buffer, [&](){
        LocalNode child("Unnamed");
        ModelIO_PackedReader_ReadNode(model, buffer, child);
        return child;
    });
}

void ModelIO::PackedReader::read(Model* model) {
    DataTemplates::push(buffer, "Model", PCK_HEADER_MODEL);

    TextureIO::TexturePackedReader texReader(buffer);
    std::vector<Material*> materialsByID = DataTemplates::read_vector<Material*>(buffer, [&](){
        int ID = DataTemplates::read_varint(buffer);
        std::string name = DataTemplates::read_string(buffer);

        int texmap_size = DataTemplates::read_varint(buffer);
        TexelByTypeMap texmap(texmap_size);
        for (int i = 0; i < texmap_size; i++) {
            MaterialTexture2D_T type = static_cast<MaterialTexture2D_T>(DataTemplates::read_varint(buffer));
            texmap[type] = DataTemplates::read_vector<Texture2D>(buffer, [&](){
                return texReader.readTexture2D();
            });
        }

        MaterialProps props = DataTemplates::read_material_props(buffer);
        int map_size = DataTemplates::read_varint(buffer);
        MetaDataMap map;
        for (int i = 0; i < map_size; i++) {
            std::string key = DataTemplates::read_string(buffer);
            int type = DataTemplates::read_varint(buffer);
            int data_len = DataTemplates::read_varint(buffer);
            std::vector<uint8_t> data(data_len);
            buffer->read(data.data(), data_len);

            M_Metadata metadata = M_Metadata(type, (char*) data.data(), data_len);
            map.insert({key, metadata});
        }
        Material* mat = new Material(ID, name, props, map);
        mat->textures.insert(texmap.begin(), texmap.end());

        return mat;
    });
    model->materialById = materialsByID;

    std::vector<NamedMesh*> meshesByID = DataTemplates::read_vector<NamedMesh*>(buffer, [&](){
        int meshID = DataTemplates::read_varint(buffer);
        std::string meshName = DataTemplates::read_string(buffer);

        std::vector<Vertex> vertices = DataTemplates::read_vector<Vertex>(buffer, [&]() {
            Vertex result;
            result.position = DataTemplates::read_glm_vec3(buffer);
            result.normal = DataTemplates::read_glm_vec3(buffer);
            result.texCoords0 = DataTemplates::read_glm_vec2(buffer);
            result.texCoords1 = DataTemplates::read_glm_vec2(buffer);
            result.color = DataTemplates::read_glm_vec4(buffer);
            result.tangent = DataTemplates::read_glm_vec3(buffer);
            for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++) {
                result.m_BoneIDs[i] = DataTemplates::read_varint(buffer);
                result.m_Weights[i] = DataTemplates::read_float(buffer);
            }
            return result;
        });
        std::vector<uint32_t> indices = DataTemplates::read_vector<uint32_t>(buffer, [&](){
            return DataTemplates::read_uint32(buffer);
        });

        int materialID = DataTemplates::read_varint(buffer);

        Mesh* mesh = new Mesh(meshID, vertices, indices);
        mesh->setMaterial(materialID == -1 ? FallbackMaterial::Default : materialsByID[materialID]);
    
        return new NamedMesh(meshName, mesh);
    });
    model->meshesById = meshesByID;
    ModelIO_PackedReader_ReadNode(model, buffer, model->rootNode);

    DataTemplates::pop(buffer, "Model", PCK_FOOTER_MODEL);
}

ModelIO::PackedWriter::PackedWriter(DataSerializer* buffer) : buffer(buffer) {}

void ModelIO_PackedWriter_WriteNode(DataSerializer *buffer, LocalNode &node) {
    DataTemplates::write_string(buffer, node.name);
    DataTemplates::write_glm_mat4(buffer, node.transform);
    DataTemplates::write_vector<NamedMesh*>(buffer, node.meshes, [&](NamedMesh* nmesh){
        DataTemplates::write_varint(buffer, nmesh->mesh->getID());
    });
    DataTemplates::write_vector<LocalNode>(buffer, node.children, [&](LocalNode child){
        ModelIO_PackedWriter_WriteNode(buffer, child);
    });
}

void ModelIO::PackedWriter::write(Model* model) {
    DataTemplates::write_uint16(buffer, PCK_HEADER_MODEL);

    TextureIO::TexturePackedWriter texWriter(buffer);
    DataTemplates::write_vector<Material*>(buffer, model->materialById, [&](Material* material){
        DataTemplates::write_varint(buffer, material->getID());
        DataTemplates::write_string(buffer, material->getName());

        DataTemplates::write_varint(buffer, material->textures.size());
        for (auto& pair : material->textures) {
            DataTemplates::write_varint(buffer, pair.first);
            DataTemplates::write_vector<Texture2D>(buffer, pair.second, [&](const Texture2D& texel){
                texWriter.writeTexture2D(texel);
            });
        }

        DataTemplates::write_material_props(buffer, material->props);
        DataTemplates::write_varint(buffer, material->metadata_map.size());
        for (auto pair : material->metadata_map) {
            M_Metadata value = pair.second;
            DataTemplates::write_string(buffer, pair.first);
            DataTemplates::write_varint(buffer, value.type);
            DataTemplates::write_varint(buffer, value.dataLength());
            buffer->write((uint8_t*) value.rawData().data(), value.dataLength());
        }
    });

    DataTemplates::write_vector<NamedMesh*>(buffer, model->meshesById, [&](NamedMesh* nmesh){
        std::string meshName = nmesh->name;
    
        Mesh* mesh = nmesh->mesh;
        std::vector<Vertex> vertices = mesh->getVertices();
        std::vector<GLuint> indices = mesh->getIndices();

        DataTemplates::write_varint(buffer, mesh->getID());
        DataTemplates::write_string(buffer, meshName);
        DataTemplates::write_vector<Vertex>(buffer, vertices, [&](const Vertex& v){
            DataTemplates::write_glm_vec3(buffer, v.position);
            DataTemplates::write_glm_vec3(buffer, v.normal);
            DataTemplates::write_glm_vec2(buffer, v.texCoords0);
            DataTemplates::write_glm_vec2(buffer, v.texCoords1);
            DataTemplates::write_glm_vec4(buffer, v.color);
            DataTemplates::write_glm_vec3(buffer, v.tangent);
            for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++) { 
                DataTemplates::write_varint(buffer, v.m_BoneIDs[i]);
                DataTemplates::write_float(buffer, v.m_Weights[i]);
            }
        });
        DataTemplates::write_vector<GLuint>(buffer, indices, [&](const GLuint& index){
            LittleEndian::write<GLuint>(buffer, index);
        });
        DataTemplates::write_varint(buffer, mesh->getMaterial()->getID());
    });

    ModelIO_PackedWriter_WriteNode(buffer, model->rootNode);
    DataTemplates::write_uint16(buffer, PCK_FOOTER_MODEL);
}