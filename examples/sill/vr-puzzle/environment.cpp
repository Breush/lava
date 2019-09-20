#include "./environment.hpp"

#include <cmath>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <lava/chamber/logger.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

#include "./brick.hpp"
#include "./serializer.hpp"
#include "./symbols.hpp"

using namespace lava;
using namespace lava::chamber;

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Skybox
    {
        engine.environmentTexture("./assets/skies/cloudy/");

        auto& skyboxEntity = engine.make<sill::GameEntity>("skybox");
        auto& skyMeshComponent = skyboxEntity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions options{.siding = sill::BoxSiding::In};
        sill::makers::boxMeshMaker(1.f, options)(skyMeshComponent);
        skyMeshComponent.category(RenderCategory::Depthless);

        auto& skyboxMaterial = engine.scene().make<magma::Material>("skybox");
        skyboxMaterial.set("useEnvironmentMap", true);
        skyboxMaterial.set("lod", 1u);
        skyMeshComponent.primitive(0, 0).material(skyboxMaterial);
    }

    // Invisible but physically present ground
    {
        auto& entity = engine.make<sill::GameEntity>("ground");
        entity.make<sill::ColliderComponent>();
        entity.get<sill::ColliderComponent>().addInfinitePlaneShape();
        entity.get<sill::PhysicsComponent>().dynamic(false);
    }
}

void levelSolved(GameState& /*gameState*/)
{
    // @note Nothing to do, really?
    // All panels have been solved!
}

void loadLevel(GameState& gameState, const std::string& levelPath)
{
    // Remove all previous entities from scene
    for (auto entity : gameState.level.entities) {
        gameState.engine->remove(*entity);
    }

    unserializeLevel(gameState, levelPath);

    if (gameState.level.name == "intro") {
        auto& clockMeshComponent = gameState.engine->findEntityByName("clock")->get<sill::MeshComponent>();
        clockMeshComponent.startAnimation("seconds-tick", -1u);
        clockMeshComponent.startAnimation("seconds-bar-tick", -1u);

        findPanelByName(gameState, "intro.waking-hall-clock-controller").onSolve([&gameState]() {
            auto wakingHall = gameState.engine->findEntityByName("waking-hall");
            wakingHall->get<sill::MeshComponent>().startAnimation("open-clock");
            wakingHall->get<sill::SoundEmitterComponent>().start("open-clock");
        });
    }
}
