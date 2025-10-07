#include "ModelInstance.hpp"

#include "MeshInstance.hpp"
#include "Model.hpp"
#include "Scene.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <vector>

using namespace syng;

ModelInstance::ModelInstance(Model* model) : model(model), root(new MeshInstance{model->rootNode}) {
    pushLeafParents(root, {});
}

void ModelInstance::pushLeafParents(MeshInstance *meI, std::vector<MeshInstance*> parentList) {
    if (!meI->getMeshes().empty()) {
        leafParents.insert({meI, parentList});
    }
    for (MeshInstance *child : meI->getChildren()) {
        parentList.push_back(meI);
        pushLeafParents(child, parentList);
        parentList.pop_back();
    }
}

glm::mat4 ModelInstance::getWorldTransform(MeshInstance* leaf, bool cacheFinalTransform) {
    if (isTransformCached(leaf) && !leaf->isMarkedDirty()) {
        return getCachedWorldTransform(leaf);
    }

    glm::mat4 world = this->transform;
    auto& ancestry = leafParents[leaf];

    int i = 0;
    for (auto node : ancestry) {
        world *= node->getTransform();
        node->markDirty(false);
    }
    world *= leaf->getTransform();
    leaf->markDirty(false);

    if (cacheFinalTransform) {
        cachedLeafFinalTransform.emplace(leaf, world);
    }
    return world;
}

glm::mat4 ModelInstance::getCachedWorldTransform(MeshInstance *leaf) {
    return isTransformCached(leaf) ? cachedLeafFinalTransform[leaf] : glm::mat4(1.0f);
}

bool ModelInstance::isTransformCached(MeshInstance *leaf) {
    return cachedLeafFinalTransform.find(leaf) != cachedLeafFinalTransform.end();
}

bool ModelInstance::shouldDiscard(Scene_T snapshot, const glm::mat4& transform) {
    bool shouldDiscard = true;
    for (auto& pair : leafParents) {
        glm::mat4 meshTransform(1.0f);
        MeshInstance *leaf = pair.first;
        if (isTransformCached(leaf) && !leaf->isMarkedDirty()) {
            meshTransform = getCachedWorldTransform(leaf);
        } else {
            meshTransform = getWorldTransform(leaf);
        }
        if (!leaf->shouldDiscard(snapshot, this->transform * meshTransform)) {
            shouldDiscard = false;
            break;
        }
    }
    return shouldDiscard;
}

void ModelInstance::setDiscard(MeshInstance* meI, bool shouldDiscard) {
    auto it = discarded.find(meI);
    if (!shouldDiscard && it != discarded.end()) {
        discarded.erase(it);
    } else if (shouldDiscard && it == discarded.end()) {
        discarded.insert(meI);
    }
}

bool ModelInstance::shouldDiscard(MeshInstance* meI) {
    return discarded.find(meI) != discarded.end();
}

MeshInstance* ModelInstance::getRoot() {
    return this->root;
}

Model* ModelInstance::getModel() {
    return this->model;
}