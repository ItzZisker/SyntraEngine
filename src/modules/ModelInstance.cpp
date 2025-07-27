#include "modules/ModelInstance.hpp"

#include "engine/RenderTable.hpp"
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
        meshInstances->add(pair.first, new MeshInstance(pair.second));
    }
}

bool ModelInstance::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    bool shouldDiscard = true;
    meshInstances->forEach([&](const std::string& key, MeshInstance* meshInstance){
        glm::mat4 worldTransform = transform * meshInstance->getTransform();
        if (!meshInstance->shouldDiscard(snapshot, worldTransform)) {
            shouldDiscard = false;
        }
    });
    return shouldDiscard;
}

void ModelInstance::renderDV(Scene_T snapshot, Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        meshInstances->forEach([&](const std::string& key, MeshInstance* meshInstance) {
            if (!meshInstance->shouldDiscard(snapshot, transform * meshInstance->getTransform())) {
                meshInstance->render(shader, screen);
            }
        });
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        MeshInstance* meshInstance = meshInstances->get(meshName);
        if (meshInstance) {
            if (!meshInstance->shouldDiscard(snapshot, transform * meshInstance->getTransform())) {
                meshInstance->render(shader, screen);
            }
        }
    }
}

void ModelInstance::render(Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        meshInstances->forEach([&](const std::string& key, MeshInstance* meshInstance) {
            meshInstance->render(shader, screen);
        });
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        MeshInstance* meshInstance = meshInstances->get(meshName);
        if (meshInstance) {
            meshInstance->render(shader, screen);
        }
    }
}

RenderTable<MeshInstance>* ModelInstance::getMeshInstances() {
    return this->meshInstances;
}

Model* ModelInstance::getModel() {
    return this->model;
}