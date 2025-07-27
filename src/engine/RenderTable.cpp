#include "RenderTable.hpp"
#include "modules/MeshInstance.hpp"
#include <engine/RenderTable.hpp>

namespace syng {
    bool RT_SORT_OPACITY(MeshInstance* a, MeshInstance* b) {
        return a->getMesh()->material.opacity > b->getMesh()->material.opacity;
    }
}