#include "MeshInstance.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glm/fwd.hpp>

#include <string>
#include <vector>

using namespace syng;

void getMinMax(LocalNode &node, glm::vec3 &min, glm::vec3 &max, bool &unset) {
    for (auto nMesh : node.meshes) {
        for (const auto& vertex : nMesh->mesh->getVertices()) {
            if (unset) {
                min = max = vertex.position;
                unset = false;
            }
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);
        }
    }
    for (auto child : node.children) {
        getMinMax(child, min, max, unset);
    }
}

MeshInstance::MeshInstance(NamedMesh namedMesh) : MeshInstance(namedMesh, {}) {}
MeshInstance::MeshInstance(NamedMesh namedMesh, Coordination coords) : MeshInstance({"ROOT", coords.getTransform()}) {}
MeshInstance::MeshInstance(LocalNode rootNode) {
    for (auto& nMesh : rootNode.meshes) {
        this->meshes.push_back(nMesh);
    }
    for (auto& child : rootNode.children) {
        MeshInstance *childM = new MeshInstance(child);
        this->children.push_back(childM);
    }
    this->name = rootNode.name;
    this->setTransform(rootNode.transform);
    
    bool unset = true;
    glm::vec3 min(0.0f), max(0.0f);
    getMinMax(rootNode, min, max, unset);
    this->bounding = AABB(min, max);
}

MeshInstance::~MeshInstance() {
    meshes.clear();
    for (auto child : children) delete child;
    children.clear();
}

void MeshInstance::markDirty(bool flag) {
    this->markedDirty = flag;
    if (flag) for (auto child : children) child->markDirty(true);
}

void MeshInstance::updateTransform() {
    this->markedDirty = true;
    Coordination::updateTransform();
}

void MeshInstance::decompose(const glm::mat4& transform) {
    this->markedDirty = true;
    Coordination::decompose(transform);
}

void MeshInstance::setTransform(const glm::mat4& transform) {
    this->markedDirty = true;
    Coordination::setTransform(transform);
}

void MeshInstance::setOrigin(const glm::vec3& newOrigin) {
    this->markedDirty = true;
    Coordination::setOrigin(newOrigin);
}

void MeshInstance::setPosition(const glm::vec3& pos) {
    this->markedDirty = true;
    Coordination::setPosition(pos);
}

void MeshInstance::addPosition(const glm::vec3& pos) {
    this->markedDirty = true;
    Coordination::addPosition(pos);
}

void MeshInstance::setScale(const glm::vec3& scale) {
    this->markedDirty = true;
    Coordination::setScale(scale);
}

void MeshInstance::setOrientation(const glm::quat& q) {
    this->markedDirty = true;
    Coordination::setOrientation(q);
}

void MeshInstance::setDirection(const glm::vec3& dir, const glm::vec3& upHint) {
    this->markedDirty = true;
    Coordination::setDirection(dir);
}

void MeshInstance::setRight(const glm::vec3& right) {
    this->markedDirty = true;
    Coordination::setRight(right);
}

void MeshInstance::setUp(const glm::vec3& up) {
    this->markedDirty = true;
    Coordination::setUp(up);
}

void MeshInstance::setEuler(float yawDeg, float pitchDeg, float rollDeg) {
    this->markedDirty = true;
    Coordination::setEuler(yawDeg, pitchDeg, rollDeg);
}

void MeshInstance::setYaw(float degrees) {
    this->markedDirty = true;
    Coordination::setYaw(degrees);
}

void MeshInstance::setPitch(float degrees) {
    this->markedDirty = true;
    Coordination::setPitch(degrees);
}

void MeshInstance::setRoll(float degrees) {
    this->markedDirty = true;
    Coordination::setRoll(degrees);
}

bool MeshInstance::isMarkedDirty() {
    return this->markedDirty;
}

std::vector<MeshInstance*>& MeshInstance::getChildren() {
    return this->children;
}

std::vector<NamedMesh*>& MeshInstance::getMeshes() {
    return this->meshes;
}

const std::string& MeshInstance::getName() {
    return this->name;
}

bool MeshInstance::hasMeshes() {
    return !this->meshes.empty();
}