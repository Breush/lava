#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    std::function<void(MeshComponent& meshComponent)> glbMeshMaker(const std::string& fileName);
}
