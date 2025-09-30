#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/world/Coordination.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace syng
{
class MeshInstance : public FrustumDiscardable, public Coordination {
private:
    RenderTable<MeshInstance> *subMeshes = new RenderTable<MeshInstance>();
    Mesh* mesh = nullptr;

    void handle(MeshInstance* meshInstance, glm::vec3& min, glm::vec3& max, bool& unset);
public:
    MeshInstance(Mesh* mesh);
    MeshInstance(Mesh* mesh, Coordination coords);
    MeshInstance(std::unordered_map<std::string, Mesh*> subMeshes);
    MeshInstance(std::unordered_map<std::string, Mesh*> subMeshes, Coordination coords);
    ~MeshInstance();

    RenderTable<MeshInstance>* getChildren();
    Mesh* getSelf();
};
}