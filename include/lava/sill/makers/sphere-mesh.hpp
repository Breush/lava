#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    std::function<void(MeshComponent& meshComponent)> sphereMeshMaker(uint32_t tessellation, float diameter);
}
