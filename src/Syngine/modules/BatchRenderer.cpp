#include "BatchRenderer.hpp"

#include "Syngine/engine/Config.hpp"
#include "Syngine/modules/Material.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/modules/Texture.hpp"
#include <iostream>
#include <ostream>

using namespace syng;

void drawMeshInstance(MeshInstance *meI, Shader& batchShader, glm::mat4 parentTransform) {
    glm::mat4 worldTransform = meI->getTransform() * parentTransform;

    if (meI->getSelf()) {
        batchShader.setMatrix4("model", worldTransform, 1, GL_FALSE);
        meI->getSelf()->draw();
    } else {
        meI->getChildren()->forEach([&](const std::string& key, MeshInstance* subMesh){
            drawMeshInstance(subMesh, batchShader, worldTransform);
        });
    }
}

void drawNonDiscardable(MeshInstance* meshInstance, Shader& batchShader, Scene_T snapshot, glm::mat4 parentTransform = glm::mat4(1.0f)) {
    if (meshInstance->getSelf()) { // ROOT
        if (!meshInstance->shouldDiscard(snapshot, meshInstance->getTransform() * parentTransform)) {
            drawMeshInstance(meshInstance, batchShader, parentTransform);
        }
    } else {
        meshInstance->getChildren()->forEach([&](const std::string key, MeshInstance *child){
            drawNonDiscardable(child, batchShader, snapshot, meshInstance->getTransform());
        });
    }
}

ModelBatchRenderer::ModelBatchRenderer(Scene *scene) : scene(scene) {}

void ModelBatchRenderer::render(Shader& batchShader, Screenbuffer screen) {
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    batchShader.use();

    for (auto& fpair : allByMaterials) {
        ModelInstance *mI = fpair.first;
        for (auto& spair : fpair.second) {
            Material *material = spair.first;

            batchShader.setFloat("F0", material->props.F0);
            batchShader.setVec3f("ior", material->props.ior);
            batchShader.setFloat("shininess", material->props.shininess);

            if (batchShader.getVariable(SHADER_REFRAC_KEY_DYNAMIC_OPACITY) == SHADER_VAL_ON) {
                batchShader.setFloat("minOpacity", material->props.minOpacity);
                batchShader.setFloat("maxOpacity", material->props.maxOpacity);
            } else {
                batchShader.setFloat("opacity", material->props.opacity);
            }

            unsigned int diffuseNr = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr = 1;
            unsigned int heightNr = 1;
            unsigned int roughNr = 1;
            unsigned int texUnit = 0;

            for (auto& pair : material->textures) {
                MaterialTexture2D_T type = pair.first;
                for (const auto& tex : pair.second) {
                    std::string number;

                    switch (type) {
                        case Texture_Diffuse: number = std::to_string(diffuseNr++); break;
                        case Texture_Specular: number = std::to_string(specularNr++); break;
                        case Texture_Normal: number = std::to_string(normalNr++); break;
                        case Texture_Height: number = std::to_string(heightNr++); break;
                        case Texture_Rough: number = std::to_string(roughNr++); break;
                    }
                    batchShader.setTexture(TEXTURE_NAME(type) + number, GL_TEXTURE_2D, texUnit++, tex.TCB);
                }
            }
            
            static std::vector<MaterialTexture2D_T> requiredTypes = {
                Texture_Diffuse,
                Texture_Specular,
                Texture_Normal
            };

            for (auto& type : requiredTypes) {
                if (!material->hasTexture(type)) {
                    GLuint TCB = FallbackTexture::get(type).TCB;
                    batchShader.setTexture(std::string(TEXTURE_NAME(type)) + "1", GL_TEXTURE_2D, texUnit++, TCB);
                }
            }

            batchShader.setBool("parallax", heightNr > 1  && material->props.hasDisplacement);
            batchShader.setBool("roughness", roughNr > 1 && material->props.hasRoughness);
        
            for (MeshInstance *meI : spair.second) {
                drawNonDiscardable(meI, batchShader, scene->getSnapshot());
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModelBatchRenderer::addMeshInstanceByMaterial(ModelInstance *mI, MeshInstance *meI) {
    if (meI->getSelf()) {
        if (allByMaterials[mI].find(meI->getSelf()->material) == allByMaterials[mI].end()) {
            allByMaterials.insert({mI, {}});
        }
        allByMaterials[mI][meI->getSelf()->material].push_back(meI);
    } else {
        meI->getChildren()->forEach([&](const std::string &key, MeshInstance *child){
            addMeshInstanceByMaterial(mI, child);
        });
    }
}

void ModelBatchRenderer::add(std::string key, ModelInstance *mI) {
    allByMaterials.insert({mI, {}});
    instances->add(key, mI);
    mI->getMeshInstances()->forEach([&](const std::string &key, MeshInstance *meI){
        addMeshInstanceByMaterial(mI, meI);
    });
}

void ModelBatchRenderer::remove(std::string key) {
    allByMaterials.erase(allByMaterials.find(instances->remove(key)));
}

std::unordered_map<Material*, std::vector<MeshInstance*>>& ModelBatchRenderer::getMeshesByMaterial(ModelInstance *mI) {
    static std::unordered_map<Material*, std::vector<MeshInstance*>> empty = {};
    return allByMaterials.find(mI) == allByMaterials.end() ? empty : allByMaterials[mI];
}

RenderTable<ModelInstance>* ModelBatchRenderer::getInstances() {
    return this->instances;
}