#include "ModelInstance.hpp"

#include "Model.hpp"
#include "Mesh.hpp"
#include "MeshInstance.hpp"
#include "Scene.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <unordered_map>

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
        if (meshInstance->getSelf()) {
            if (!meshInstance->shouldDiscard(snapshot, meshInstance->getSelf()->getParentToNodeTransform() * transform)) {
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

void ModelInstance::setDiscard(std::string mIKey, bool shouldDiscard) {
    discardedInstances[mIKey] = shouldDiscard;
}

bool ModelInstance::shouldDiscard(std::string mIKey) {
    auto p = discardedInstances.find(mIKey);
    return p != discardedInstances.end() && p->second;
}

RenderTable<MeshInstance>* ModelInstance::getMeshInstances() {
    return this->meshInstances;
}

Model* ModelInstance::getModel() {
    return this->model;
}