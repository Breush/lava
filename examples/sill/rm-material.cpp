/**
 * Shows how the roughness/metallic material works.
 */

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    ashe::Application app;
    auto& engine = app.engine();

    auto& entity = engine.make<sill::GameEntity>();

    float roughness = 0.f;
    float roughnessSpeed = 0.2f;
    auto& material = engine.make<sill::Material>("roughness-metallic");
    material.set("roughnessFactor", roughness);
    material.set("metallicFactor", 1.f);

    auto& meshComponent = entity.make<sill::MeshComponent>();
    sill::makers::sphereMeshMaker(32u, 1.f)(meshComponent);
    meshComponent.node(0u).mesh->primitive(0u).material(material);

    auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
    behaviorComponent.onUpdate([&](float dt) {
        roughness += roughnessSpeed * dt;
        if (roughness <= 0.f) {
            roughness = 0.f;
            roughnessSpeed = std::abs(roughnessSpeed);
        }
        else if (roughness >= 1.f) {
            roughness = 1.f;
            roughnessSpeed = -std::abs(roughnessSpeed);
        }
        material.set("roughnessFactor", roughness);
    });

    engine.run();

    return EXIT_SUCCESS;
}
