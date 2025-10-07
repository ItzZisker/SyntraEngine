#pragma once

#include "Syngine/engine/RenderTable.hpp"

#include "Material.hpp"
#include "ModelInstance.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"
#include "Syngine/modules/Model.hpp"

#include <unordered_map>
#include <vector>

namespace syng
{

using ByInstanceMap = std::unordered_map<MeshInstance*, std::vector<NamedMesh>>;
using BatchMap = std::unordered_map<Material*, ByInstanceMap>;

class UIBatchRenderer : public ShaderRenderable
{
private:
    Scene *scene;

};

class ModelBatchRenderer : public ShaderRenderable
{
private:
    Scene *scene;

    std::unordered_map<ModelInstance*, BatchMap> allByMaterials;
    RenderTable<ModelInstance>* instances = new RenderTable<ModelInstance>;

    void addMeshInstanceByMaterial(ModelInstance *mI, MeshInstance *meI);
public:
    ModelBatchRenderer(Scene *scene);

    void render(Shader& batchShader, Screenbuffer screen) override;

    void add(std::string key, ModelInstance *mI);
    void remove(std::string key);

    BatchMap& getMeshesByMaterial(ModelInstance *mI);
};

};
