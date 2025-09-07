#pragma once

#include "MeshInstance.hpp"
#include "Screenbuffer.hpp"
#include "Model.hpp"
#include "Scene.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>
#include <string>

namespace syng
{
class ModelInstance : public Discardable, public ShaderRenderable {
private:
    Model* model;
    RenderTable<MeshInstance>* meshInstances;
    std::unordered_map<std::string, bool> discardedInstances;
public:
    ModelInstance(Model* model);

    ~ModelInstance();

    void renderDV(Scene_T snapshot, Shader& shader, Screenbuffer screen);
    void render(Shader& shader, Screenbuffer screen = {}) override;

    void setDiscard(std::string mIKey, bool shouldDiscard);

    bool shouldDiscard(std::string mIKey);
    bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) override;

    RenderTable<MeshInstance>* getMeshInstances();

    Model* getModel();
};
}