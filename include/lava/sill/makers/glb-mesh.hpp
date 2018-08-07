#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    std::function<void(MeshComponent&)> glbMeshMaker(const std::string& fileName);
}
