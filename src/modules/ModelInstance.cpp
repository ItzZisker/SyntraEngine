#include "modules/ModelInstance.hpp"

#include "engine/RenderTable.hpp"
#include "modules/MeshInstance.hpp"
#include "modules/Model.hpp"
#include "modules/Scene.hpp"
#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include <iostream>
#include <modules/Mesh.hpp>
#include <unordered_map>
#include <utils/GameUtils.hpp>

using namespace syng;

ModelInstance::ModelInstance(Model* model) : model(model) {
    meshInstances = new RenderTable<MeshInstance>();
    for (auto& basePair : model->meshGroups) {
        std::string baseName = basePair.first;
        std::unordered_map<std::string, Mesh*> baseMap = basePair.second;

        meshInstances->add(baseName, new MeshInstance(baseMap));
    }
}

ModelInstance::~ModelInstance() {
    meshInstances->wipeAll();
    delete meshInstances;
}

bool ModelInstance::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    bool shouldDiscard = true;
    meshInstances->forEach([&](const std::string& key, MeshInstance* meshInstance){
        Mesh* self = meshInstance->getSelf();
        if (self) {
            if (!meshInstance->shouldDiscard(snapshot, self->getParentToNodeTransform() * transform)) {
                shouldDiscard = false;
            }
        } else {
            if (!meshInstance->shouldDiscard(snapshot, transform)) {
                shouldDiscard = false;
            }
        }
    });
    return shouldDiscard;
}

void renderNonDiscardable(MeshInstance* meshInstance, Shader shader, Scene_T snapshot, Screenbuffer screen, glm::mat4 parentTransform = glm::mat4(1.0f)) {
    Mesh* self = meshInstance->getSelf();
    if (self) {
        if (!meshInstance->shouldDiscard(snapshot, meshInstance->getTransform())) {
            meshInstance->render(shader, screen, parentTransform);
        }
    } else {
        meshInstance->getChildren()->forEach([&](const std::string key, MeshInstance *child){
            renderNonDiscardable(child, shader, snapshot, screen, meshInstance->getTransform());
        });
    }
}

void ModelInstance::renderDV(Scene_T snapshot, Shader shader, Screenbuffer screen) {
    if (!model->loaded) {
        return;
    }
    if (model->renderable_meshes.empty()) {
        meshInstances->forEach([&](const std::string& key, MeshInstance* meshInstance) {
            renderNonDiscardable(meshInstance, shader, snapshot, screen);
        });
        return;
    }
    for (const std::string& meshName : model->renderable_meshes) {
        MeshInstance* meshInstance = meshInstances->get(meshName);
        if (meshInstance) {
            renderNonDiscardable(meshInstance, shader, snapshot, screen);
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