#pragma once

#include "Syngine/engine/RenderTable.hpp"

#include "Material.hpp"
#include "ModelInstance.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"

#include <unordered_map>

namespace syng
{

class UIBatchRenderer : public ShaderRenderable
{
private:
    Scene *scene;

};

class ModelBatchRenderer : public ShaderRenderable
{
private:
    Scene *scene;

    std::unordered_map<ModelInstance*, std::unordered_map<Material*, std::vector<MeshInstance*>>> allByMaterials;
    RenderTable<ModelInstance>* instances = new RenderTable<ModelInstance>;

    void addMeshInstanceByMaterial(ModelInstance *mI, MeshInstance *meI);
public:
    ModelBatchRenderer(Scene *scene);

    void render(Shader& batchShader, Screenbuffer screen) override;

    void add(std::string key, ModelInstance *mI);
    void remove(std::string key);

    std::unordered_map<Material*, std::vector<MeshInstance*>>& getMeshesByMaterial(ModelInstance *mI);
};

};
