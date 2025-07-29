#include "RenderTable.hpp"
#include "modules/MeshInstance.hpp"
#include <engine/RenderTable.hpp>
#include <algorithm>

namespace syng {
    bool RT_SORT_OPACITY(MeshInstance* a, MeshInstance* b) {
        auto aMat = a->getMesh()->material;
        auto bMat = b->getMesh()->material;

        float aOMin = std::min(aMat.opacity, aMat.maxOpacity);
        float bOMin = std::min(bMat.opacity, bMat.maxOpacity);

        return aOMin > bOMin;
    }
}