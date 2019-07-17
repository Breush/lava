/**
 * Shows how to setup a VR environment using magma rendering-engine.
 */

#include <lava/magma.hpp>

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    // Render engine: the global manager.
    magma::RenderEngine engine;

    // The VR device
    auto& vrTarget = engine.make<magma::VrRenderTarget>();

    // @fixme Should have a way to position the center of the world for VR.

    // Render scene: holds what has to be drawn.
    auto& scene = engine.make<magma::RenderScene>();
    scene.rendererType(magma::RendererType::Forward); // Deferred might be too much for VR

    // Tell the VR render target to exist within this scene.
    vrTarget.bindScene(scene);

    // Lights.
    {
        auto& light = scene.make<magma::Light>();
        magma::PointLightController lightController(light);
        lightController.translation({0.2, 0.4, 2});
        lightController.radius(10.f);
    }

    // Some demo mesh
    auto& firstMesh = ashe::makeCube(scene, 0.5);
    firstMesh.translate({0, 0, 0.5});

    // @fixme No way to break out while no companion window.
    while (true) {
        // Render the scene.
        engine.update();
        engine.draw();
    }

    return EXIT_SUCCESS;
}
