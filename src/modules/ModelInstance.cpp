#include "Syngine/modules/ModelInstance.hpp"

#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Shader.hpp"
#include "Syngine/world/WorldObject.hpp"
#include "glm/fwd.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

ModelInstance::ModelInstance(Model* model, CoordinatedObject coords) : model(model) {
    setTransform(coords.getTransform());

    bool init = true;
    glm::vec3 min, max;

    for (auto& pair : model->meshes) {
        Mesh* mesh = pair.second;

        if (init) {
            min = mesh->vertices[0].position;
            max = mesh->vertices[0].position;
            init = false;
        }
        for (const auto& vertex : mesh->vertices) {
            min = glm::min(min, vertex.position);
            max = glm::max(max, vertex.position);
        }
        meshInstances.emplace(pair.first, MeshInstance(mesh));
    }
    AABB = BoundingBox(min, max);
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