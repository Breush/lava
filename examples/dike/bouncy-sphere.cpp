/**
 * Small physics demo.
 */

#include <lava/dike.hpp>

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace lava;

int main(void)
{
    dike::PhysicsEngine engine;
    engine.gravity({0, 0, -10});

    // Ground
    auto& plane = engine.make<dike::RigidBody>();
    plane.addInfinitePlaneShape();
    plane.dynamic(false);

    // Falling sphere
    auto& sphere = engine.make<dike::RigidBody>();
    sphere.addSphereShape({0.f, 0.f, 0.f}, 0.1f);

    lava::Transform transform;
    transform.translation = {0, 0, 1.f};
    sphere.transform(transform);

    // Simulating the world
    for (auto i = 0u; i < 45u; i++) {
        uint32_t distance = std::round(sphere.transform().translation.z * 80);
        for (auto j = 1u; j < distance; ++j) {
            std::cout << ' ';
        }
        std::cout << '*' << std::endl;
        engine.update(1 / 20.f);
    }
}
