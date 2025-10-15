#pragma once

#include "MeshInstance.hpp"
#include "Model.hpp"

using namespace syng;

namespace syng
{

class BatchedMesh : public GLVertexElement<Vertex> {
private:
    Material *material = FallbackMaterial::Default;
public:
    BatchedMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    void setMaterial(Material *mat);
    Material *getMaterial();

    void render(Shader& shader, Screenbuffer screen, glm::mat4 transform);
    void init(CacheApproach::VRAM_Approach = CacheApproach::Sequential);
};

class StaticVAOBatchModel : public Model {
public:


    // Root node path consists of the path to the node which batching starts from, separated by '/'
    void uploadBatchedVertices(const std::string &rootNodePath, CacheApproach::VRAM_Approach approach = CacheApproach::Sequential);
};

}