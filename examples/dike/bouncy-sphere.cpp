/**
 * Small physics demo.
 */

#include <lava/dike.hpp>

#include <cmath>
#include <iostream>

using namespace lava;

int main(void)
{
    dike::PhysicsEngine engine;
    engine.gravity({0, 0, -10});

    // Ground
    engine.make<dike::PlaneStaticRigidBody>(glm::vec3{0, 0, 1});

    // Falling sphere
    auto& sphere = engine.make<dike::SphereRigidBody>(0.1f);
    sphere.mass(1);
    sphere.translate({0, 0, 1.f});

    // Simulating the world
    for (auto i = 0u; i < 45u; i++) {
        uint32_t distance = std::round(sphere.translation().z * 80);
        for (auto j = 1u; j < distance; ++j) {
            std::cout << ' ';
        }
        std::cout << '*' << std::endl;
        engine.update(1 / 20.f);
    }
}
