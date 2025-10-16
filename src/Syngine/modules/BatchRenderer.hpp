#pragma once

#include "Syngine/engine/RenderTable.hpp"

#include "Material.hpp"
#include "ModelInstance.hpp"
#include "Scene.hpp"
#include "Screenbuffer.hpp"
#include "Shader.hpp"
#include "Syngine/modules/Model.hpp"

#include <vector>

namespace syng
{

class UIAtlasBatchRenderer : public ShaderRenderable
{
private:
    Scene *scene;
};

struct RenderBatch {
    int sceneShaderID;
    Material* material;
    ModelInstance *modelInstance;
    MeshInstance *meshInstance;
    std::vector<NamedMesh> mesh;
};

using SortFunc = std::function<bool(const RenderBatch& a, const RenderBatch& b)>;
using ByInstanceMap = std::unordered_map<MeshInstance*, std::vector<NamedMesh>>;
using NamedMeshByMaterial = std::unordered_map<Material*, ByInstanceMap>;

inline SortFunc DEFAULT_BATCH_SORT = [](const RenderBatch& a, const RenderBatch& b) {
    if (a.sceneShaderID != b.sceneShaderID) return a.sceneShaderID < b.sceneShaderID;
    if (a.material != b.material) return a.material < b.material;
    return a.modelInstance < b.modelInstance;
};

class MaterialBatchRenderer : public ShaderRenderable, public DepthRenderable
{
private:
    Scene *scene;

    std::vector<RenderBatch> batches;
    RenderTable<ModelInstance>* instances = new RenderTable<ModelInstance>;

    void addMeshInstancesByMaterial(NamedMeshByMaterial &namedMeshesByMaterial, ModelInstance *mI, MeshInstance *rootMei);
public:
    MaterialBatchRenderer(Scene *scene);

    void renderDepth(Shader& depthShader, Screenbuffer screen) override;
    void render(Shader& batchShader, Screenbuffer screen) override;

    void add(ModelInstance *mI);
    void remove(ModelInstance *mI);
    void sort(SortFunc func = DEFAULT_BATCH_SORT);
};

};
