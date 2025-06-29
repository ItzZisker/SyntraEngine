#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MeshInstance : public FrustumDiscardable, public Coordination, public ShaderRenderable {
private:
    Mesh* mesh;
public:
    MeshInstance(Mesh* mesh);

    MeshInstance(Mesh* mesh, Coordination coords);

    void render(Shader shader, int FBO) override;

    Mesh* getMesh();
};