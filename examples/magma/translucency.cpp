/**
 * Shows how translucency works using magma rendering-engine.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | Translucency");

    // Blue mesh
    {
        auto& blueMaterial = app.scene().make<magma::Material>("ashe");
        blueMaterial.set("color", {0.f, 0.f, 1.f, 0.3f});

        auto& blueMesh = app.makePlane({1, 1});
        blueMesh.rotate({0.f, 1.f, 0.f}, 1.6f);
        blueMesh.material(blueMaterial);
    }

    // Red mesh
    {
        auto& redMaterial = app.scene().make<magma::Material>("ashe");
        redMaterial.set("color", {1.f, 0.f, 0.f, 0.5f});

        auto& redMesh = app.makePlane({1, 1});
        redMesh.translate({-0.25f, 0.f, 0.f});
        redMesh.material(redMaterial);
    }

    app.camera().translation({2.f, 2.f, 2.f});
    app.camera().target({0.f, 0.f, 0.f});

    app.light().radius(1.1f);

    app.run();

    return EXIT_SUCCESS;
}
