#pragma once

#include <functional>

namespace lava::sill {
    class IMesh;
}

namespace lava::sill::makers {
    // Final bounding dimensions of the tore will be [bigDiameter + smallDiameter; bigDiameter + smallDiameter; smallDiameter].
    std::function<uint32_t(IMesh&)> toreMeshMaker(uint32_t bigTessellation, float bigDiameter,
                                                  uint32_t smallTessellation, float smallDiameter);
}
