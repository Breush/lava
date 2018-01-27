/**
 * Small physics demo.
 */

#include <lava/dike.hpp>

#include <iomanip>
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
    sphere.positionAdd({0, 0, 1.f});

    // Simulating the world
    for (auto i = 0u; i < 40u; i++) {
        std::cout << "Time: " << std::fixed << std::setprecision(3) << i / 5.f << " | ";
        std::cout << "Sphere height: " << sphere.position().z << std::endl;
        engine.update(1 / 20.f);
    }
}
