#pragma once

#include "MeshInstance.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ModelInstance : public DiscardableObject, public CoordinatedObject, public ShaderRenderable {
public:
    Model* model;
    std::unordered_map<std::string, MeshInstance> meshInstances;

    ModelInstance(Model* model, CoordinatedObject coords = CoordinatedObject());

    void render(Shader shader, int FBO) override;
};