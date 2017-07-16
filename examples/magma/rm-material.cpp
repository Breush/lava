/**
 * Shows how the roughness/metallic material works.
 */

#include "../ashe.hpp"

using namespace lava;

constexpr const uint8_t SPHERES_SIDE_COUNT = 6u;

int main(void)
{
    ashe::Application app("ashe - magma | RmMaterial");

    // Create a bunch of spheres
    const float spheresSideCountRange = SPHERES_SIDE_COUNT - 1u;
    for (auto i = 0u; i < SPHERES_SIDE_COUNT; ++i) {
        for (auto j = 0u; j < SPHERES_SIDE_COUNT; ++j) {
            auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
            auto& material = app.engine().make<magma::RmMaterial>();
            material.roughness(i / spheresSideCountRange);
            material.metallic(j / spheresSideCountRange);
            sphereMesh.material(material);
            sphereMesh.positionAdd({i * 1.1f, j * 1.1f, 0.5f});
        }
    }

    // Move light
    app.light().position({-2.f, -2.f, 5.f});

    // Move camera
    app.camera().position({-3.f, -3.f, 5.f});
    app.camera().target({SPHERES_SIDE_COUNT * 0.5f, SPHERES_SIDE_COUNT * 0.5f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
