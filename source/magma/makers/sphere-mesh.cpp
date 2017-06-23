#include <lava/magma/makers/sphere-mesh.hpp>

#include <lava/magma/mesh.hpp>

#include <iostream>

using namespace lava;

std::function<void(Mesh& mesh)> makers::sphereMeshMaker(uint32_t tessellation)
{
    return [](Mesh& mesh) {
        mesh.verticesCount(4);
        mesh.verticesPositions({{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}});
        mesh.indices({0, 1, 2, 2, 3, 0});
    };
}
