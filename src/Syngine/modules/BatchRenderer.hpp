#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Material.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"

#include <unordered_map>
#include <map>

namespace syng
{

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

    RenderTable<ModelInstance>* getInstances();
    std::unordered_map<Material*, std::vector<MeshInstance*>>& getMeshesByMaterial(ModelInstance *mI);
};

};
