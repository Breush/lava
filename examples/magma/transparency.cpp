/**
 * Shows how transparency works using magma rendering-engine.
 */

#include "../ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | Transparency");

    // Blue mesh
    {
        auto& blueMaterial = app.scene().make<magma::RmMaterial>();
        blueMaterial.baseColor({0u, 0u, 255u, 120u}, 1u, 1u, 4u);

        auto& blueMesh = app.scene().make(magma::makers::planeMeshMaker({1, 1}));
        blueMesh.rotationAdd({0.f, 1.f, 0.f}, 1.6f);
        blueMesh.material(blueMaterial);
    }

    // Red mesh
    {
        auto& redMaterial = app.scene().make<magma::RmMaterial>();
        redMaterial.baseColor({255u, 0u, 0u, 120u}, 1u, 1u, 4u);

        auto& redMesh = app.scene().make(magma::makers::planeMeshMaker({1, 1}));
        redMesh.positionAdd({-0.25f, 0.f, 0.f});
        redMesh.material(redMaterial);
    }

    app.camera().position({2.f, 2.f, 2.f});
    app.camera().target({0.f, 0.f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
