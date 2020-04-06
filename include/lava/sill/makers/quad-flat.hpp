#pragma once

#include <functional>
namespace lava::sill {
    class FlatComponent;
    struct FlatNode;
}

namespace lava::sill::makers {
    std::function<FlatNode&(FlatComponent&)> quadFlatMaker(float sidesLength);
    std::function<FlatNode&(FlatComponent&)> quadFlatMaker(const glm::vec2& extent);
}
