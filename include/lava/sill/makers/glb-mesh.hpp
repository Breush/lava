#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
    struct MeshNode;
}

namespace lava::sill::makers {
    std::function<MeshNode&(MeshComponent&)> glbMeshMaker(const std::string& fileName);
}
