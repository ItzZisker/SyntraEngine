#pragma once

#include "Screenbuffer.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Mesh.hpp"
#include "world/WorldObject.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace syng
{
class MeshInstance : public FrustumDiscardable, public Coordination, public ShaderRenderable {
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

    void render(Shader shader, Screenbuffer screen, glm::mat4 parentTransform);

    void render(Shader shader, Screenbuffer screen = {}) override;

    RenderTable<MeshInstance>* getChildren();

    Mesh* getSelf();
};
}