#include "modules/MeshInstance.hpp"

#include "modules/Screenbuffer.hpp"
#include "modules/Shader.hpp"
#include "world/WorldObject.hpp"
#include <modules/Mesh.hpp>
#include <utils/GameUtils.hpp>

using namespace syng;

MeshInstance::MeshInstance(Mesh* mesh) : MeshInstance(mesh, Coordination(glm::mat4(1.0f))) {}

MeshInstance::MeshInstance(Mesh* mesh, Coordination coords) : mesh(mesh) {
    glm::vec3 min = mesh->vertices[0].position;
    glm::vec3 max = mesh->vertices[0].position;

    for (const auto& vertex : mesh->vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }
    bounding = AABB(min, max);
    setTransform(coords.getTransform());
}

void MeshInstance::render(Shader shader, Screenbuffer screen) {
    mesh->render(shader, screen, getTransform());
}

Mesh* MeshInstance::getMesh() {
    return this->mesh;
}