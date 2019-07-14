/**
 * Shows the different kinds of shadows.
 */

#include "./ashe.hpp"

using namespace lava;

// @todo Have something similar in lava/core.
float random01()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

int main(void)
{
    ashe::Application app("ashe - magma | Shadows");

    // Ground
    {
        app.makePlane({10, 10});
    }

    const auto minScaling = 0.3;
    const auto maxScaling = 0.5;

    // Structure
    for (auto i = 0u; i < 5u; ++i) {
        auto& mesh = app.makeCube(1.f);
        mesh.scale(minScaling + (maxScaling - minScaling) * random01());
        mesh.translate({random01() - 1.f, 2.f * random01() - 1.f, random01()});
        mesh.rotate({random01(), random01(), random01()}, 3.14f / 4.f * random01());
    }

    // Move the main camera
    app.cameraController().origin({-1.5f, 3.1f, 3.f});
    app.cameraController().target({-0.8f, -0.2f, 0.2f});

    // A second camera to test multiple views with shadow maps
    auto& camera = app.scene().make<magma::Camera>(Extent2d{400u, 300u});
    magma::OrbitCameraController cameraController(camera);
    cameraController.origin({-1.5f, 3.1f, 3.f});
    cameraController.target({-0.8f, -0.2f, 0.2f});
    app.engine().addView(camera, app.windowRenderTarget(), Viewport{0.5, 0, 0.5, 0.5});

    auto t = 0.f;
    app.run([&](float dt) {
        t += dt;
        app.light().direction({std::cos(t), std::sin(t), -0.5f});
    });

    return EXIT_SUCCESS;
}
