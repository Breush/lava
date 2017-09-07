#pragma once

#include <functional>

#include <lava/magma/extent.hpp>

namespace lava::magma {
    class Mesh;
}

namespace lava::magma::makers {
    std::function<void(Mesh& mesh)> planeMeshMaker(Extent2d dimensions);
}
