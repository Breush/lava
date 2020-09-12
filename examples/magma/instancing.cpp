/**
 * Shows how instancing works using magma rendering-engine.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app("ashe - magma | Instancing");
    
    // One cube, instanced 51*51*51 = 132'651 times
    auto& cubeMesh = app.makeCube(1);
    constexpr auto sideLength = 51;
    constexpr auto halfSideLength = (sideLength - 1) / 2;

    cubeMesh.reserveInstancesCount(sideLength * sideLength * sideLength);
    for (auto x = -halfSideLength; x <= halfSideLength; ++x) {
        for (auto y = -halfSideLength; y <= halfSideLength; ++y) {
            for (auto z = -halfSideLength; z <= halfSideLength; ++z) {
                auto instanceIndex = cubeMesh.addInstance();
                cubeMesh.translate(glm::vec3{1.75f * x, 1.5f * y, 1.25f * z}, instanceIndex);
            }
        }
    }

    app.cameraController().origin(glm::vec3{2.f * (halfSideLength + 1)});
    app.cameraController().target({0.f, 0.f, 0.f});

    app.light().shadowsEnabled(false);
    app.run();

    return EXIT_SUCCESS;
}
