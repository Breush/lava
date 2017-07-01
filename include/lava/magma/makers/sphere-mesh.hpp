#pragma once

#include <functional>

namespace lava::magma {
    class Mesh;
}

namespace lava::magma::makers {
    std::function<void(Mesh& mesh)> sphereMeshMaker(uint32_t tessellation, float radius);
}
