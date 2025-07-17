#pragma once

#include "Screenbuffer.hpp"
#include "engine/RenderTable.hpp"
#include "modules/Mesh.hpp"
#include "world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MeshInstance : public FrustumDiscardable, public Coordination, public ShaderRenderable {
private:
    Mesh* mesh;
public:
    MeshInstance(Mesh* mesh);

    MeshInstance(Mesh* mesh, Coordination coords);

    void render(Shader shader, Screenbuffer screen = {}) override;

    Mesh* getMesh();
};