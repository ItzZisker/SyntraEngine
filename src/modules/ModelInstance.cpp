#include "Syngine/modules/ModelInstance.hpp"

#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

ModelInstance::ModelInstance(Model* model, Coordination coords) : model(model) {
    setTransform(coords.getTransform());

    for (auto& pair : model->meshes) {
        meshInstances.emplace(pair.first, MeshInstance(pair.second));
    }
}

bool ModelInstance::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    for (auto& meshInstance : meshInstances) {
        glm::mat4 worldTransform = transform * meshInstance.second.getTransform();
        if (!meshInstance.second.shouldDiscard(snapshot, worldTransform)) {
            return false;
        }
    }
    return true;
}

void ModelInstance::renderDV(Scene_T snapshot, Shader shader, int FBO) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            MeshInstance meshInstance = pair.second;

            if (!meshInstance.shouldDiscard(snapshot, transform * meshInstance.getTransform())) {
                meshInstance.render(shader, FBO);
            }
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            MeshInstance meshInstance = pair->second;

            if (!meshInstance.shouldDiscard(snapshot, transform * meshInstance.getTransform())) {
                meshInstance.render(shader, FBO);
            }
        }
    }
}

void ModelInstance::render(Shader shader, int FBO) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            pair.second.render(shader, FBO);
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            pair->second.render(shader, FBO);
        }
    }
}