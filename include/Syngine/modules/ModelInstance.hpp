#pragma once

#include "MeshInstance.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ModelInstance : public Discardable, public Coordination, public ShaderRenderable {
private:
    Model* model;
public:
    std::unordered_map<std::string, MeshInstance> meshInstances;

    ModelInstance(Model* model, Coordination coords = Coordination());

    void render(Shader shader, int FBO) override;

    bool shouldDiscard(Scene* scene, const glm::mat4& transform) override;
};