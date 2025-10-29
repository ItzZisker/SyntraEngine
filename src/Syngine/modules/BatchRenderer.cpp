#include "BatchRenderer.hpp"

#include "Syngine/engine/Config.hpp"

#include "Material.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "MeshInstance.hpp"
#include "Model.hpp"
#include "ModelInstance.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"

#include <algorithm>
#include <unordered_map>

using namespace syng;

void drawNonDiscardable(Mesh *mesh, MeshInstance* parent, Shader& batchShader, Scene_T snapshot, glm::mat4 finalTransform) {
    if (!parent->shouldDiscard(snapshot, finalTransform)) {
        batchShader.setMatrix4("model", finalTransform, 1, GL_FALSE);
        mesh->draw();
    }
}

MaterialBatchRenderer::MaterialBatchRenderer(Scene *scene) : scene(scene) {}

void renderBatchDepth(Scene_T snapshot, Shader &depthShader, const RenderBatch& batch) {
    ModelInstance *mI = batch.modelInstance;
    MeshInstance *parent = batch.meshInstance;
    glm::mat4 finalTransform = mI->getTransform() * mI->getWorldTransform(parent);
    for (auto *nmesh : batch.mesh) {
        drawNonDiscardable(nmesh->mesh, parent, depthShader, snapshot, finalTransform);
    }
}

void MaterialBatchRenderer::renderDepth(Shader &depthShader, Screenbuffer screen) {
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    Scene_T snapshot = scene->getSnapshot();
    for (auto& batch : batches) {
        renderBatchDepth(snapshot, depthShader, batch);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderBatch(Scene_T snapshot, Shader &batchShader, const RenderBatch& batch) {
    Material *material = batch.material;

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
            batchShader.setTexture(TEXTURE_NAME(type) + number, GL_TEXTURE_2D, texUnit++, tex.getTCB());
        }
    }
    
    static std::vector<MaterialTexture2D_T> requiredTypes = {
        Texture_Diffuse,
        Texture_Specular,
        Texture_Normal
    };

    for (auto& type : requiredTypes) {
        if (!material->hasTexture(type)) {
            GLuint TCB = FallbackTexture::get(type).getTCB();
            batchShader.setTexture(std::string(TEXTURE_NAME(type)) + "1", GL_TEXTURE_2D, texUnit++, TCB);
        }
    }

    batchShader.setBool("parallax", heightNr > 1  && material->props.hasDisplacement);
    batchShader.setBool("roughness", roughNr > 1 && material->props.hasRoughness);

    ModelInstance *mI = batch.modelInstance;
    MeshInstance *parent = batch.meshInstance;

    glm::mat4 finalTransform = mI->getTransform() * mI->getWorldTransform(parent);
    for (auto *nmesh : batch.mesh) {
        drawNonDiscardable(nmesh->mesh, parent, batchShader, snapshot, finalTransform);
    }
}

void MaterialBatchRenderer::render(Shader& batchShader, Screenbuffer screen) {
    glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
    batchShader.use();
    Scene_T snapshot = scene->getSnapshot();
    for (auto& batch : batches) renderBatch(snapshot, batchShader, batch);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MaterialBatchRenderer::addMeshInstancesByMaterial(NamedMeshByMaterial &namedMeshesByMaterial, ModelInstance *mI, MeshInstance *meI) {
    for (auto& nmesh : meI->getMeshes()) {
        Material *material = nmesh->mesh->getMaterial();
        if (namedMeshesByMaterial.find(material) == namedMeshesByMaterial.end()) {
            namedMeshesByMaterial.insert({material, {}});
        }
        if (namedMeshesByMaterial[material].find(meI) == namedMeshesByMaterial[material].end()) {
            namedMeshesByMaterial[material].insert({meI, {}});
        }
        namedMeshesByMaterial[material][meI].push_back(nmesh);
    }
    for (auto child : meI->getChildren()) {
        addMeshInstancesByMaterial(namedMeshesByMaterial, mI, child);
    }
}

void MaterialBatchRenderer::add(ModelInstance *mI) {
    NamedMeshByMaterial namedMeshesByMaterial;
    addMeshInstancesByMaterial(namedMeshesByMaterial, mI, mI->getRoot());
    for (auto& fpair : namedMeshesByMaterial) {
        Material *material = fpair.first;
        ByInstanceMap &byInstance = fpair.second;
        for (auto& spair : byInstance) {
            MeshInstance *meI = spair.first;
            batches.push_back({0, material, mI, meI, {spair.second}});
        }
    }
    namedMeshesByMaterial.clear();
}

void MaterialBatchRenderer::remove(ModelInstance *mI) {
    batches.erase(std::remove_if(
        batches.begin(),
        batches.end(),
        [mI](const RenderBatch& b) { return b.modelInstance == mI; }
    ));
}

void MaterialBatchRenderer::sort(SortFunc func) {
    std::sort(batches.begin(), batches.end(), func);
}