#pragma once

#include <functional>
#include <lava/core/extent.hpp>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    std::function<void(MeshComponent&)> planeMeshMaker(Extent2d dimensions);
}
