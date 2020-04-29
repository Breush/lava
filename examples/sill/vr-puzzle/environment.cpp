#include "./environment.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/logger.hpp>
#include <lava/chamber/math.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./game-state.hpp"
#include "./serializer.hpp"
#include "./symbols.hpp"

using namespace lava;
using namespace lava::chamber;

namespace {
    void levelMaterialGhostFactor(GameState& gameState, const std::string& materialName, float ghostFator)
    {
        for (auto& generic : gameState.level.generics) {
            auto& entity = generic->entity();
            if (!entity.has<sill::MeshComponent>()) continue;
            for (auto& node : entity.get<sill::MeshComponent>().nodes()) {
                if (node.meshGroup == nullptr) continue;
                for (auto primitive : node.meshGroup->primitives()) {
                    auto material = primitive->material().get();
                    if (material == nullptr) continue;
                    if (material->name() != materialName) continue;
                    auto& animationComponent = entity.ensure<sill::AnimationComponent>();
                    animationComponent.start(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", 0.5f);
                    animationComponent.target(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", ghostFator);
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

        auto& skyboxEntity = engine.make<sill::GameEntity>("skybox");
        auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions options;
        options.siding = sill::BoxSiding::In;
        sill::makers::boxMeshMaker(1.f, options)(skyMeshComponent);
        skyMeshComponent.category(RenderCategory::Depthless);

        auto skyboxMaterial = engine.scene().makeMaterial("skybox");
        skyboxMaterial->set("useEnvironmentMap", true);
        skyboxMaterial->set("lod", 1u);
        skyMeshComponent.primitive(0, 0).material(skyboxMaterial);
    }

    // Water
    {
        auto& entity = engine.make<sill::GameEntity>("water");
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker(512.f)(meshComponent);
        meshComponent.category(RenderCategory::Translucent);
        entity.get<sill::TransformComponent>().translate({0.f, 0.f, -0.3f});

        auto waveTexture = engine.scene().makeTexture();
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
        gameState.terrain.entity = gameState.engine->findEntityByName("island-1");
        gameState.terrain.entity->ensure<sill::PhysicsComponent>().dynamic(false);
        gameState.terrain.entity->ensure<sill::ColliderComponent>().addMeshShape();

        // @note Automatically replace all wood materials in the world.
        levelMaterialGhostFactor(gameState, "wood", 1.f);

        // @fixme Have these stored in JSON somehow?
        findPanelByName(gameState, "intro.bridge-controller")->onSolvedChanged([&gameState](bool solved) {
            levelMaterialGhostFactor(gameState, "wood", (solved) ? 0.f : 1.f);
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
    }
}
