#include "modules/ModelInstance.hpp"

#include "modules/MeshInstance.hpp"
#include "modules/Model.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include <modules/Mesh.hpp>
#include <utils/GameUtils.hpp>

using namespace syng;

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

void ModelInstance::renderDV(Scene_T snapshot, Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            MeshInstance meshInstance = pair.second;

            if (!meshInstance.shouldDiscard(snapshot, transform * meshInstance.getTransform())) {
                meshInstance.render(shader, screen);
            }
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            MeshInstance meshInstance = pair->second;

            if (!meshInstance.shouldDiscard(snapshot, transform * meshInstance.getTransform())) {
                meshInstance.render(shader, screen);
            }
        }
    }
}

void ModelInstance::render(Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        for (auto& pair : meshInstances) {
            pair.second.render(shader, screen);
        }
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        auto pair = meshInstances.find(meshName);

        if (pair != meshInstances.end()) {
            pair->second.render(shader, screen);
        }
    }
}

Model* ModelInstance::getModel() {
    return this->model;
}