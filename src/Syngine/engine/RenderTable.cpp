#include "RenderTable.hpp"

#include "Syngine/modules/MeshInstance.hpp"

namespace syng {
    bool RT_SORT_OPACITY_SUB_MESH(Mesh* a, Mesh* b) {
        auto aMat = a->material;
        auto bMat = b->material;

        float aOMin = std::min(aMat.opacity, aMat.maxOpacity);
        float bOMin = std::min(bMat.opacity, bMat.maxOpacity);

        return aOMin > bOMin;
    }
    bool RT_SORT_OPACITY_GROUP_MESH(MeshInstance* a, MeshInstance* b) {
        float aMaxOpacity = 0.0f;
        float bMaxOpacity = 0.0f;

        a->getChildren()->forEach([&](std::string key, MeshInstance* child) {
            auto& mat = child->getSelf()->material;
            float op = std::min(mat.opacity, mat.maxOpacity);
            aMaxOpacity = std::max(aMaxOpacity, op);
        });
        b->getChildren()->forEach([&](std::string key, MeshInstance* child) {
            auto& mat = child->getSelf()->material;
            float op = std::min(mat.opacity, mat.maxOpacity);
            bMaxOpacity = std::max(bMaxOpacity, op);
        });
        return aMaxOpacity > bMaxOpacity;
    }    
}