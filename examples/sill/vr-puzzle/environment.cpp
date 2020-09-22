#include "./environment.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <lava/chamber/logger.hpp>
#include <lava/chamber/math.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./game-state.hpp"
#include "./objects/pedestal.hpp"
#include "./serializer.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    // @note Generic entities might have colliders expressed
    // within their mesh component. These are the mesh nodes named ":collider".
    void genericCollidersBuildPhysicsFromMesh(GameState& gameState)
    {
        for (auto generic : gameState.level.generics) {
            auto& entity = generic->entity();
            if (!entity.has<sill::MeshComponent>()) continue;
            for (auto& node : entity.get<sill::MeshComponent>().nodes()) {
                if (node.group == nullptr) continue;
                if (node.name != ":collider") continue;

                entity.ensure<sill::PhysicsComponent>().dynamic(false);
                entity.ensure<sill::ColliderComponent>().addMeshNodeShape(node);
            }
        }
    }

    void genericCollidersVisible(GameState& gameState, bool enabled)
    {
        for (auto generic : gameState.level.generics) {
            auto& entity = generic->entity();
            if (!entity.has<sill::MeshComponent>()) continue;
            for (auto& node : entity.get<sill::MeshComponent>().nodes()) {
                if (node.group == nullptr) continue;
                if (node.name != ":collider") continue;
                for (auto primitive : node.group->primitives()) {
                    primitive->enabled(enabled);
                }
            }
        }
    }

    void levelMaterialGhostFactor(GameState& gameState, const std::string& materialName, float ghostFactor)
    {
        for (auto& object : gameState.level.objects) {
            auto& entity = object->entity();
            if (!entity.has<sill::MeshComponent>()) continue;

            // @todo We probably don't want to let that logic here...
            if (object->kind() == "pedestal" && object->as<Pedestal>().material() == materialName) {
                object->as<Pedestal>().powered(ghostFactor == 0.f);
            }

            for (auto& node : entity.get<sill::MeshComponent>().nodes()) {
                if (node.group == nullptr) continue;
                for (auto primitive : node.group->primitives()) {
                    auto material = primitive->material().get();
                    if (material == nullptr) continue;
                    if (material->name() != materialName) continue;
                    auto& animationComponent = entity.ensure<sill::AnimationComponent>();
                    animationComponent.start(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", 0.5f);
                    animationComponent.target(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", ghostFactor);
                }
            }
        }
    }
}

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Skybox
    {
        engine.environmentTexture("./assets/skies/cloudy/");

        auto& skyboxEntity = engine.make<sill::Entity>("skybox");
        auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions options;
        options.siding = sill::BoxSiding::In;
        sill::makers::boxMeshMaker(1.f, options)(skyMeshComponent);
        skyMeshComponent.renderCategory(RenderCategory::Depthless);

        auto skyboxMaterial = engine.scene().makeMaterial("skybox");
        skyboxMaterial->set("useEnvironmentMap", true);
        skyboxMaterial->set("lod", 1u);
        skyMeshComponent.primitive(0, 0).material(skyboxMaterial);
    }

    // Water
    {
        auto& entity = engine.make<sill::Entity>("water");
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker(512.f)(meshComponent);
        meshComponent.renderCategory(RenderCategory::Translucent);
        entity.get<sill::TransformComponent>().translate({0.f, 0.f, -0.3f});

        auto waveTexture = engine.renderEngine().makeTexture();
        waveTexture->loadFromFile("./assets/textures/vr-puzzle/water.png");

        auto material = engine.scene().makeMaterial("water");
        material->set("waveMap", waveTexture);
        meshComponent.primitive(0, 0).material(material);

        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([material = material.get()](float dt) {
            static float time = 0.f;
            time += dt;
            material->set("time", time);
        });
    }
}

void levelSolved(GameState& /*gameState*/)
{
    // @note Nothing to do, really?
    // All panels have been solved!
}

void loadLevel(GameState& gameState, const std::string& levelPath)
{
    unserializeLevel(gameState, levelPath);

    if (gameState.level.name == "intro") {
        // Build collider components from the ":collider" meshes.
        genericCollidersBuildPhysicsFromMesh(gameState);
        genericCollidersVisible(gameState, false);

        // Automatically decide what is ghosted here...
        levelMaterialGhostFactor(gameState, "rock", 0.f);
        levelMaterialGhostFactor(gameState, "wood", 1.f);

        // @fixme Have these stored in JSON somehow?
        findPanelByName(gameState, "intro.bridge-controller")->onSolvedChanged([&gameState](bool solved) {
            levelMaterialGhostFactor(gameState, "wood", (solved) ? 0.f : 1.f);
            findGenericByName(gameState, "bridge")->walkable(solved);
        });
        findPanelByName(gameState, "intro.easy-solo")->onSolvedChanged([&gameState](bool solved) {
            findBarrierByName(gameState, "intro.easy-duo")->powered(solved);
        });
        findPanelByName(gameState, "intro.easy-duo-1")->onSolvedChanged([&gameState](bool solved) {
            if (findPanelByName(gameState, "intro.easy-duo-2")->solved()) {
                findBarrierByName(gameState, "intro.hard-duo")->powered(solved);
            }
        });
        findPanelByName(gameState, "intro.easy-duo-2")->onSolvedChanged([&gameState](bool solved) {
            if (findPanelByName(gameState, "intro.easy-duo-1")->solved()) {
                findBarrierByName(gameState, "intro.hard-duo")->powered(solved);
            }
        });
        findPanelByName(gameState, "intro.big-key")->onSolvedChanged([&gameState](bool solved) {
            if (!solved) return;

            auto& boat = *findGenericByName(gameState, "env/boat");
            auto transform = boat.transform().worldTransform();
            transform.translation.z -= 5.5f;

            boat.animation().start(sill::AnimationFlag::WorldTransform, 2.f);
            boat.animation().target(sill::AnimationFlag::WorldTransform, transform);
        });
    }
}

float distanceToTerrain(GameState& gameState, const Ray& ray, Generic** pGeneric, float maxDistance)
{
    auto distance = gameState.terrain.entity->distanceFrom(ray, sill::PickPrecision::Collider);

    if (pGeneric != nullptr) {
        *pGeneric = nullptr;
    }

    // Check generic colliders
    for (auto generic : gameState.level.generics) {
        auto& entity = generic->entity();
        if (!entity.has<sill::ColliderComponent>()) continue;
        auto genericDistance = entity.distanceFrom(ray, sill::PickPrecision::Collider);
        if (genericDistance != 0.f && (distance == 0.f || genericDistance < distance)) {
            distance = genericDistance;
            if (pGeneric != nullptr) {
                *pGeneric = generic;
            }
        }
    }

    if (distance > maxDistance) {
        if (pGeneric != nullptr) {
            *pGeneric = nullptr;
        }
        return 0.f;
    }

    return distance;
}
