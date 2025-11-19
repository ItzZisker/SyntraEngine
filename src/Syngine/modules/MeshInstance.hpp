#pragma once

#include "Model.hpp"

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace syng
{

class MeshInstance : public FrustumDiscardable, public Coordination {
private:
    std::string name = "NONE";

    std::vector<MeshInstance*> children = {};
    std::vector<NamedMesh*> meshes = {};

    bool markedDirty = false;

    void handle(LocalNode& node, glm::vec3& min, glm::vec3& max, bool& unset);
public:
    MeshInstance(NamedMesh* namedMesh);
    MeshInstance(NamedMesh* namedMesh, Coordination coords);
    MeshInstance(LocalNode rootNode);
    ~MeshInstance();

    void markDirty(bool flag);
    bool isMarkedDirty();

    void updateTransform() override;

    std::vector<MeshInstance*>& getChildren();
    std::vector<NamedMesh*>& getMeshes();

    const std::string& getName();
    bool hasMeshes();
};

}