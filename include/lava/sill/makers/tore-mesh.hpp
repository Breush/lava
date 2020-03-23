#pragma once

#include <functional>

namespace lava::sill {
    class MeshComponent;
}

namespace lava::sill::makers {
    // Final bounding dimensions of the tore will be [bigDiameter + smallDiameter; bigDiameter + smallDiameter; smallDiameter].
    std::function<void(MeshComponent&)> toreMeshMaker(uint32_t bigTessellation, float bigDiameter,
                                                      uint32_t smallTessellation, float smallDiameter);
}
