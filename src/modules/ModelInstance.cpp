#include "Syngine/modules/ModelInstance.hpp"

#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Shader.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

ModelInstance::ModelInstance(Model* model, CoordinatedObject coords) : model(model) {
    setTransform(coords.getTransform());
    for (auto& mesh : model->meshes) {
        meshInstances[mesh.first] = MeshInstance(mesh.second);
    }
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