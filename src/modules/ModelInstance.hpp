#pragma once

#include "MeshInstance.hpp"
#include "Screenbuffer.hpp"
#include "modules/Model.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Scene.hpp"
#include "world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace syng
{
class ModelInstance : public Discardable, public Coordination, public ShaderRenderable {
private:
    Model* model;
public:
    std::unordered_map<std::string, MeshInstance> meshInstances;

    ModelInstance(Model* model, Coordination coords = Coordination());

    void renderDV(Scene_T snapshot, Shader shader, Screenbuffer screen);

    void render(Shader shader, Screenbuffer screen = {}) override;

    bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) override;

    Model* getModel();
};
}