#include "BatchRenderer.hpp"

#include "Syngine/engine/Config.hpp"

#include "Mesh.hpp"
#include "MeshInstance.hpp"
#include "Model.hpp"
#include "Texture.hpp"

#include <vector>

using namespace syng;

void drawNonDiscardable(NamedMesh &subMesh, MeshInstance* parent, Shader& batchShader, Scene_T snapshot, glm::mat4 finalTransform) {
   if (!parent->shouldDiscard(snapshot, finalTransform)) {
        batchShader.setMatrix4("model", finalTransform, 1, GL_FALSE);
        subMesh.mesh->draw();
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

            ByInstanceMap &byInstance = spair.second;
            for (auto& tpair : byInstance) {
                MeshInstance *parent = tpair.first;
                glm::mat4 finalTransform = mI->getTransform() * mI->getWorldTransform(parent);
                for (auto& nmesh : tpair.second)
                    drawNonDiscardable(nmesh, parent, batchShader, scene->getSnapshot(), finalTransform);
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModelBatchRenderer::addMeshInstanceByMaterial(ModelInstance *mI, MeshInstance *meI) {
    for (auto& nmesh : meI->getMeshes()) {
        Material *material = nmesh.mesh->getMaterial();
        if (allByMaterials[mI].find(material) == allByMaterials[mI].end()) {
            allByMaterials[mI].insert({material, {}});;
        }
        if (allByMaterials[mI][material].find(meI) == allByMaterials[mI][material].end()) {
            allByMaterials[mI][material].insert({meI, {}});
        }
        allByMaterials[mI][material][meI].push_back(nmesh);
    }
    for (auto child : meI->getChildren()) {
        addMeshInstanceByMaterial(mI, child);
    }
}

void ModelBatchRenderer::add(std::string key, ModelInstance *mI) {
    allByMaterials.insert({mI, {}});
    instances->add(key, mI);
    addMeshInstanceByMaterial(mI, mI->getRoot());
}

void ModelBatchRenderer::remove(std::string key) {
    ModelInstance *mI = instances->remove(key);
    allByMaterials.erase(allByMaterials.find(mI));
}

BatchMap& ModelBatchRenderer::getMeshesByMaterial(ModelInstance *mI) {
    static BatchMap empty = {};
    return allByMaterials.find(mI) == allByMaterials.end() ? empty : allByMaterials[mI];
}