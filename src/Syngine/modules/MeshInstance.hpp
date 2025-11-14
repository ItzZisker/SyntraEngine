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
    void setScale(const glm::vec3& scale) override;
    void setOrientation(const glm::quat& q) override;

    void setDirection(const glm::vec3& dir, const glm::vec3& upHint) override;
    void setRight(const glm::vec3& newRight) override;
    void setUp(const glm::vec3& up) override;

    void setEuler(float yawDeg, float pitchDeg, float rollDeg) override;
    void setYaw(float degrees) override;
    void setPitch(float degrees) override;
    void setRoll(float degrees) override;

    std::vector<MeshInstance*>& getChildren();
    std::vector<NamedMesh*>& getMeshes();

    const std::string& getName();
    bool hasMeshes();
};

}