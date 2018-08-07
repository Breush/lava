/**
 * Shows how the roughness/metallic material works.
 */

#include <lava/sill.hpp>

using namespace lava;

int main(void)
{
    std::vector<std::string> matcapFiles = {
        "./assets/matcaps/black-pearl.png", "./assets/matcaps/blue-glint.png", "./assets/matcaps/blue-pearl.png",
        "./assets/matcaps/bubblegum.png",   "./assets/matcaps/cold-steel.png", "./assets/matcaps/hot-metal.png",
        "./assets/matcaps/metal.png",       "./assets/matcaps/sunset.png",     "./assets/matcaps/toon-blue.png",
        "./assets/matcaps/warm-steel.png",  "./assets/matcaps/warmth.png",
    };

    sill::GameEngine engine;

    // Create a sphere for each material
    float x = -5.5f;
    for (auto& matcapFile : matcapFiles) {
        auto& texture = engine.make<sill::Texture>(matcapFile);
        auto& material = engine.make<sill::Material>("matcap");
        material.set("texture", texture);

        auto& sphereEntity = engine.make<sill::GameEntity>();
        auto& sphereMeshComponent = sphereEntity.make<sill::MeshComponent>();
        sill::makers::sphereMeshMaker(32u, 1.f)(sphereMeshComponent);
        sphereMeshComponent.node(0u).mesh->primitive(0u).material(material);

        sphereEntity.get<sill::TransformComponent>().translate({x, 0.f, 0.});
        x += 1.f;
    }

    engine.run();

    return EXIT_SUCCESS;
}
