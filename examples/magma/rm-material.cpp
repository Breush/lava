/**
 * Shows how the roughness/metallic material works.
 */

#include "../ashe.hpp"

using namespace lava;

constexpr const uint8_t SPHERES_SIDE_COUNT = 6u;

int main(void)
{
    ashe::Application app("ashe - magma | RmMaterial");

    /*auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
    auto& material = app.engine().make<magma::RmMaterial>();
    material.metallic(0.f);
    material.roughness(0.f);
    sphereMesh.material(material);
    sphereMesh.positionAdd({0.f, 0.f, 0.5f});*/

    auto& mesh = app.engine().make<magma::Mesh>("./assets/models/corset.glb");
    auto& material = mesh.material();
    material.metallic(0.f);
    material.roughness(0.f);

    /*for (auto i = 0u; i < SPHERES_SIDE_COUNT; ++i) {
        for (auto j = 0u; j < SPHERES_SIDE_COUNT; ++j) {
            auto& sphereMesh = app.engine().make(magma::makers::sphereMeshMaker(32, 0.5));
            sphereMesh.material(app.engine().make<magma::RmMaterial>());
            sphereMesh.positionAdd({i * 1.f, j * 1.f, 0.5f});
        }
    }*/

    app.run([&material](const crater::Event& event) {
        switch (event.type) {
        case crater::Event::KeyPressed: {
            if (event.key.which == crater::input::Key::M) {
                material.metallic(material.metallic() + 0.1f);
                std::cout << "Metallic: " << material.metallic() << std::endl;
            }
            else if (event.key.which == crater::input::Key::R) {
                material.roughness(material.roughness() + 0.1f);
                std::cout << "Roughness: " << material.roughness() << std::endl;
            }
            break;
        }
        default: break;
        }
    });

    return EXIT_SUCCESS;
}
