/**
 * Shows how the roughness/metallic material works.
 */

#include "../ashe.hpp"

using namespace lava;

constexpr const uint8_t SPHERES_SIDE_COUNT = 6u;

int main(void)
{
    ashe::Application app("ashe - magma | RmMaterial");

    auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
    sphereMesh.material(app.engine().make<magma::RmMaterial>());
    sphereMesh.positionAdd({0.f, 0.f, 0.5f});

    auto& sphereMesh2 = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
    sphereMesh2.material(app.engine().make<magma::RmMaterial>());
    sphereMesh2.positionAdd({1.f, 0.f, 0.5f});

    /*for (auto i = 0u; i < SPHERES_SIDE_COUNT; ++i) {
        for (auto j = 0u; j < SPHERES_SIDE_COUNT; ++j) {
            auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
            sphereMesh.material(app.engine().make<magma::RmMaterial>());
            sphereMesh.positionAdd({i * 1.f, j * 1.f, 0.5f});
        }
    }*/

    app.run();

    return EXIT_SUCCESS;
}
