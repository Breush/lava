#pragma once

#include <functional>

namespace lava::sill {
    class IMesh;
    struct MeshNode;
}

namespace lava::sill::makers {
    std::function<uint32_t(IMesh&)> glbMeshMaker(const std::string& fileName);
}
