#include <lava/magma/makers/sphere-mesh.hpp>

#include <lava/chamber/math/constants.hpp>
#include <lava/magma/mesh.hpp>

#include <iostream> // @fixme

using namespace lava;

std::function<void(Mesh& mesh)> makers::sphereMeshMaker(uint32_t tessellation)
{
    return [&](Mesh& mesh) {
        auto latitude = 0.f;
        auto step = math::TWO_PI / (tessellation - 1u);
        for (auto i = 0u; i < tessellation; ++i) {
            auto longitude = 0.f;
            for (auto j = 0u; j < tessellation; ++j) {
                std::cout << latitude << " " << longitude << std::endl;
                longitude += step;
            }
            latitude += step;
        }

        mesh.verticesCount(4);
        mesh.verticesPositions({{-1, -1, 0.5}, {-1, 1, 0.5}, {1, 1, 0.5}, {1, -1, 0.5}});
        mesh.indices({0, 2, 1, 3, 2, 0});
    };
}
