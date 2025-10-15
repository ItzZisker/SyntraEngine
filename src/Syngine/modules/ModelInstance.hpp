#pragma once

#include "MeshInstance.hpp"
#include "Model.hpp"
#include "Scene.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include "Syngine/ports/GLPort.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <set>
#include <unordered_map>

namespace syng
{

class ModelInstance : public Coordination, public Discardable {
private:
    Model* model;

    MeshInstance* root;
    std::unordered_map<MeshInstance*, std::vector<MeshInstance*>> leafParents;
    std::unordered_map<MeshInstance*, glm::mat4> cachedLeafFinalTransform;

    std::set<MeshInstance*> discarded;

    void pushLeafParents(MeshInstance *meI, std::vector<MeshInstance*> parentList);
public:
    ModelInstance(Model* model);

    glm::mat4 getWorldTransform(MeshInstance* leaf, bool cacheFinalTransform = true);
    glm::mat4 getCachedWorldTransform(MeshInstance *leaf);

    bool isTransformCached(MeshInstance *leaf);

    void setDiscard(MeshInstance* meI, bool shouldDiscard);

    bool shouldDiscard(MeshInstance* meI);
    bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) override;

    MeshInstance* getRoot();
    Model* getModel();
};

}