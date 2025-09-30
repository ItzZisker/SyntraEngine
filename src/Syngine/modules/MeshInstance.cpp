#include "MeshInstance.hpp"
#include "Mesh.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glm/fwd.hpp>

#include <string>

using namespace syng;

MeshInstance::MeshInstance(Mesh* mesh) : MeshInstance(mesh, {}) {}
MeshInstance::MeshInstance(Mesh* mesh, Coordination coords) : MeshInstance({{"ROOT", mesh}}, coords) {}
MeshInstance::MeshInstance(std::unordered_map<std::string, Mesh*> subMeshes) : MeshInstance(subMeshes, {}) {}
MeshInstance::MeshInstance(std::unordered_map<std::string, Mesh*> subMeshes, Coordination coords) {
    if (subMeshes.find("ROOT") != subMeshes.end()) {
        this->mesh = subMeshes["ROOT"];
        setTransform(mesh->getParentToNodeTransform() * coords.getTransform());
    } else {
        for (const auto& pair : subMeshes) {
            this->subMeshes->add(pair.first, new MeshInstance(pair.second));
        }
        setTransform(coords.getTransform());
    }
    bool unset = true;
    glm::vec3 min(0.0f), max(0.0f);
    handle(this, min, max, unset);
    bounding = AABB(min, max);
}

MeshInstance::~MeshInstance() {
    mesh = nullptr;
    subMeshes->wipeAll();
    delete subMeshes;
}

void MeshInstance::handle(MeshInstance* meshInstance, glm::vec3& min, glm::vec3& max, bool& unset) {
    if (meshInstance->getSelf()) {
        for (const auto& vertex : meshInstance->getSelf()->getVertices()) {
            if (unset) {
                min = max = vertex.position;
                unset = false;
            }
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);
        }
    } else {
        meshInstance->getChildren()->forEach([&](const std::string& key, MeshInstance* child){
            handle(child, min, max, unset);
        });
    }
}

RenderTable<MeshInstance>* MeshInstance::getChildren() {
    return this->subMeshes;
}

Mesh* MeshInstance::getSelf() {
    return this->mesh;
}