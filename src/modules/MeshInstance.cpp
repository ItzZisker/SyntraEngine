#include "Syngine/modules/MeshInstance.hpp"

#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

MeshInstance::MeshInstance(Mesh* mesh) : MeshInstance(mesh, CoordinatedObject(mesh->getTransform())) {}

MeshInstance::MeshInstance(Mesh* mesh, CoordinatedObject coords) : mesh(mesh) {
    AABB = mesh->getAABB();
    setTransform(coords.getTransform());
}

void MeshInstance::render(Shader shader, int FBO) {
    mesh->render(shader, FBO, transform);
}