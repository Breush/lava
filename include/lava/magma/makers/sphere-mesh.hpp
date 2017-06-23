#pragma once

#include <functional>

namespace lava {
    class Mesh;
}

namespace lava::makers {
    std::function<void(Mesh& mesh)> sphereMeshMaker(uint32_t tessellation);
}
