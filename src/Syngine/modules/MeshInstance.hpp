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
    std::vector<NamedMesh> meshes = {};

    bool markedDirty = false;

    void handle(LocalNode& node, glm::vec3& min, glm::vec3& max, bool& unset);
protected:
    void updateTransform() override;
    void decompose(const glm::mat4& transform) override;
public:
    MeshInstance(NamedMesh namedMesh);
    MeshInstance(NamedMesh namedMesh, Coordination coords);
    MeshInstance(LocalNode rootNode);
    ~MeshInstance();

    void markDirty(bool flag);
    bool isMarkedDirty();

    void setTransform(const glm::mat4& transform) override;
    void setOrigin(const glm::vec3& newOrigin) override;
    void setPosition(const glm::vec3& pos) override;
    void addPosition(const glm::vec3& pos) override;
    void setUp(const glm::vec3& up) override;
    void setDirection(const glm::vec3& dir) override;
    void setScale(glm::vec3 scale) override;

    std::vector<MeshInstance*>& getChildren();
    std::vector<NamedMesh>& getMeshes();

    const std::string& getName();
    bool hasMeshes();
};

}