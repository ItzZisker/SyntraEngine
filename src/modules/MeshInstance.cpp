#include "Syngine/modules/MeshInstance.hpp"

#include "Syngine/modules/Shader.hpp"
#include <Syngine/modules/Mesh.hpp>
#include <Syngine/utils/GameUtils.hpp>

MeshInstance::MeshInstance(Mesh* mesh, CoordinatedObject coords) : mesh(mesh) {
    setTransform(coords.getTransform());
}

void MeshInstance::render(Shader shader, int FBO) {
    if (!mesh->loaded) return;

    shader.use();
    shader.setMatrix4("model", transform, 1, GL_FALSE);

    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    if (mesh->textures.empty()) {
        glBindTexture(GL_TEXTURE_2D, getDefaultWhiteTexture());
    }

    for (unsigned int i = 0; i < mesh->textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = mesh->textures[i].type;
        
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);

        shader.setInt(name + number, i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}