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

    // Structure
    for (auto i = 0u; i < 5u; ++i) {
        auto& mesh = app.makeCube(0.3f + 0.2f * random01());
        // @fixme Invert the two of these will make a very different
        // transform matrix. We might want to store each independently
        // and recompose the transform matrix (keeping it TRS).
        mesh.translate({2.f * random01() - 1.f, 3.f * random01() - 1.5f, random01()});
        mesh.rotate({random01(), random01(), random01()}, 3.14f / 4.f * random01());
    }

    // Camera
    app.camera().translation({2.f, 2.f, 2.f});
    app.camera().target({0.f, 0.f, 0.f});

    app.run();

    return EXIT_SUCCESS;
}
