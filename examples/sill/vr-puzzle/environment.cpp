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

    // Water
    {
        auto& entity = engine.make<sill::GameEntity>("water");
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::planeMeshMaker(512.f)(meshComponent);
        meshComponent.category(RenderCategory::Translucent);
        entity.get<sill::TransformComponent>().translate({0.f, 0.f, -0.3f});

        auto& waveTexture = engine.scene().make<magma::Texture>();
        waveTexture.loadFromFile("./assets/textures/vr-puzzle/water.png");

        auto& material = engine.scene().make<magma::Material>("water");
        material.set("waveMap", waveTexture);
        meshComponent.primitive(0, 0).material(material);

        auto& behaviorComponent = entity.make<sill::BehaviorComponent>();
        behaviorComponent.onUpdate([&](float dt) {
            static float time = 0.f;
            time += dt;
            material.set("time", time);
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

        auto& clockEntity = *gameState.engine->findEntityByName("clock");
        auto& clockMeshComponent = clockEntity.get<sill::MeshComponent>();
        clockMeshComponent.startAnimation("seconds-tick", -1u);
        clockMeshComponent.startAnimation("seconds-bar-tick", -1u);

        auto baseTransform = clockMeshComponent.node(2).transform();

        clockEntity.make<sill::BehaviorComponent>().onUpdate([&clockMeshComponent, baseTransform](float dt) {
            static float timePassed = 60.f;
            timePassed += dt;

            // We effectively update the clock each minute.
            if (timePassed >= 60.f) {
                timePassed -= 60.f;

                auto theTime = time(NULL);
                auto aTime = localtime(&theTime); // @note Don't free, according to spec'.
                auto minutes = 60 * aTime->tm_hour + aTime->tm_min;
                float rotation = (minutes / 720.f) * math::TWO_PI;

                clockMeshComponent.node(2).transform(glm::rotate(baseTransform, rotation, {0, 0, 1}));
            }
        });

        // @fixme Have these stored in JSON somehow?
        findPanelByName(gameState, "intro.waking-hall-clock-controller")->onSolve([&gameState]() {
            // auto wakingHall = gameState.engine->findEntityByName("waking-hall");
            // wakingHall->get<sill::MeshComponent>().startAnimation("open-clock");
            // wakingHall->get<sill::SoundEmitterComponent>().start("open-clock");
        });
        findPanelByName(gameState, "intro.easy-solo")->onSolve([&gameState]() {
            findBarrierByName(gameState, "intro.easy-duo")->powered(true);
        });
        findPanelByName(gameState, "intro.easy-duo-1")->onSolve([&gameState]() {
            if (findPanelByName(gameState, "intro.easy-duo-2")->solved()) {
                findBarrierByName(gameState, "intro.hard-duo")->powered(true);
            }
        });
        findPanelByName(gameState, "intro.easy-duo-2")->onSolve([&gameState]() {
            if (findPanelByName(gameState, "intro.easy-duo-1")->solved()) {
                findBarrierByName(gameState, "intro.hard-duo")->powered(true);
            }
        });
    }
}
