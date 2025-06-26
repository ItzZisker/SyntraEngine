#include "Syngine/modules/ModelInstance.hpp"

#include "Syngine/modules/Model.hpp"
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

bool ModelInstance::shouldDiscard(Scene* scene, const glm::mat4& transform) {
    for (auto& meshInstance : meshInstances) {
        glm::mat4 worldTransform = transform * meshInstance.second.getTransform();
        if (!meshInstance.second.shouldDiscard(scene, worldTransform)) {
            return false;
        }
    }
    return true;
}

void ModelInstance::render(Shader shader, int FBO) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& meshInstance : meshInstances) {
            meshInstance.second.render(shader, FBO);
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto found = meshInstances.find(meshName);

        if (found != meshInstances.end()) {
            found->second.render(shader, FBO);
        }
    }
}