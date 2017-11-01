/**
 * Shows how the roughness/metallic material works.
 */

#include <lava/sill.hpp>

using namespace lava;

constexpr const uint8_t SPHERES_SIDE_COUNT = 6u;

int main(void)
{
    sill::GameEngine engine;

    // @todo TextureManager ?
    auto& skyTexture = engine.make<sill::Texture>("./assets/skies/panorama-spherical_sichuan.jpg");

    // Sky
    // @todo Set sky in the engine
    // @todo No depth buffer and always centered on the camera
    auto& skyboxEntity = engine.make<sill::GameEntity>();
    auto& skyboxMaterial = engine.make<sill::Material>("sky");
    skyboxMaterial.set("texture", skyTexture);
    auto& skyboxMeshComponent = skyboxEntity.make<sill::MeshComponent>();
    skyboxMeshComponent.material(skyboxMaterial);

    sill::makers::SphereMeshOptions sphereMeshOptions;
    sphereMeshOptions.siding = sill::SphereSiding::IN;
    sphereMeshOptions.coordinatesSystem = sill::SphereCoordinatesSystem::PANORAMA_SPHERICAL;
    sill::makers::sphereMeshMaker(64u, 100.f, sphereMeshOptions)(skyboxMeshComponent);

    // Create a bunch of spheres
    const float spheresSideCountRange = SPHERES_SIDE_COUNT - 1u;
    for (auto i = 0u; i < SPHERES_SIDE_COUNT; ++i) {
        for (auto j = 0u; j < SPHERES_SIDE_COUNT; ++j) {
            auto& sphereEntity = engine.make<sill::GameEntity>();

            auto& material = engine.make<sill::Material>("roughness-metallic");
            material.set("roughnessFactor", i / spheresSideCountRange);
            material.set("metallicFactor", j / spheresSideCountRange);

            auto& sphereMeshComponent = sphereEntity.make<sill::MeshComponent>();
            sill::makers::sphereMeshMaker(32u, 1.f)(sphereMeshComponent);
            sphereMeshComponent.material(material);

            sphereEntity.get<sill::TransformComponent>().positionAdd({i * 1.1f, j * 1.1f, 0.5f});
        }
    }

    engine.run();

    return EXIT_SUCCESS;
}
