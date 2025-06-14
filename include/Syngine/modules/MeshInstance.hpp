#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/world/WorldObject.hpp"
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MeshInstance : public DiscardableObject, public CoordinatedObject, public ShaderRenderable {
public:
    Mesh* mesh;

    MeshInstance(Mesh* mesh);

    MeshInstance(Mesh* mesh, CoordinatedObject coords);

    void render(Shader shader, int FBO) override;
};