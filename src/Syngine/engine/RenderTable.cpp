#include "RenderTable.hpp"

#include "Syngine/modules/MeshInstance.hpp"

namespace syng {
    bool RT_SORT_OPACITY_SUB_MESH(Mesh* a, Mesh* b) {
        auto aMat = a->getMaterial();
        auto bMat = b->getMaterial();

        float aOMin = std::min(aMat->props.opacity, aMat->props.maxOpacity);
        float bOMin = std::min(bMat->props.opacity, bMat->props.maxOpacity);

        return aOMin > bOMin;
    }
    // TODO: Sorted color blending for meshes
    // bool RT_SORT_OPACITY_GROUP_MESH(MeshInstance* a, MeshInstance* b) {
    //     float aMaxOpacity = 0.0f;
    //     float bMaxOpacity = 0.0f;

    //     for (auto child : a->getChildren()) {
    //         auto& mat = child->getSelf()->material;
    //         float op = std::min(mat->props.opacity, mat->props.maxOpacity);
    //         aMaxOpacity = std::max(aMaxOpacity, op);
    //     }

    //     a->getChildren()->forEach([&](std::string key, MeshInstance* child) {
    //     });
    //     b->getChildren()->forEach([&](std::string key, MeshInstance* child) {
    //         auto& mat = child->getSelf()->material;
    //         float op = std::min(mat->props.opacity, mat->props.maxOpacity);
    //         bMaxOpacity = std::max(bMaxOpacity, op);
    //     });
    //     return aMaxOpacity > bMaxOpacity;
    // }
}