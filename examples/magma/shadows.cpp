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

    // Camera
    app.camera().translation({2.f, 2.f, 2.f});
    app.camera().target({0.f, 0.f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
