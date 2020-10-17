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

    bool entitySubstanceGhostFactor(sill::Entity& entity, const std::string& substanceName, float ghostFactor)
    {
        bool completed = (ghostFactor == 0.f);
        bool concerned = false;

        for (auto& node : entity.get<sill::MeshComponent>().nodes()) {
            if (node.group == nullptr) continue;
            for (auto primitive : node.group->primitives()) {
                auto material = primitive->material().get();
                if (material == nullptr) continue;

                if (substanceName.empty() || material->name() == substanceName) {
                    auto& animationComponent = entity.ensure<sill::AnimationComponent>();
                    animationComponent.start(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", 0.5f);
                    animationComponent.target(sill::AnimationFlag::MaterialUniform, *material, "ghostFactor", ghostFactor);
                    concerned = true;
                } else {
                    completed = completed && (material->get_float("ghostFactor") != 0.f);
                }
            }
        }

        return concerned && completed;
    }
}

void setupEnvironment(GameState& gameState)
{
    auto& engine = *gameState.engine;

    // Skybox
    {
        engine.environmentTexture("./assets/skies/cloudy/");

        auto& entity = engine.make<sill::Entity>("skybox");
        auto& meshComponent = entity.make<sill::MeshComponent>();
        sill::makers::BoxMeshOptions options;
        options.siding = sill::BoxSiding::In;
        sill::makers::boxMeshMaker(1.f, options)(meshComponent);
        meshComponent.renderCategory(RenderCategory::Depthless);

        auto material = engine.scene().makeMaterial("skybox");
        material->set("useEnvironmentMap", true);
        material->set("lod", 1u);
        meshComponent.primitive(0, 0).material(material);

        gameState.level.environment.sky = &entity;
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

        gameState.level.environment.sea = &entity;
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

        // Starting point of the island
        // @fixme Have their "powered" state to on directly!
        revealSubstance(gameState, "sky");

        // @todo Have this stored in JSON somehow? To do that, we would need a scripting language...
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

// ----- Substance

void freeSubstance(GameState& gameState, const std::string& substanceName, float ghostFactor)
{
    if (substanceName == "sky") {
        entitySubstanceGhostFactor(*gameState.level.environment.sky, "", ghostFactor);
        return;
    }
    if (substanceName == "sea") {
        entitySubstanceGhostFactor(*gameState.level.environment.sea, "", ghostFactor);
        return;
    }

    for (auto& object : gameState.level.objects) {
        auto& entity = object->entity();
        if (!entity.has<sill::MeshComponent>()) continue;
        if (object->kind() != "generic") continue;

        bool concernedAndCompleted = entitySubstanceGhostFactor(entity, substanceName, ghostFactor);
        if (concernedAndCompleted && entity.has<sill::ColliderComponent>()) {
            object->as<Generic>().walkable(ghostFactor == 0.f);
        }
    }
}

void revealSubstance(GameState& gameState, const std::string& substanceName, float ghostFactor)
{
    for (auto& object : gameState.level.objects) {
        auto& entity = object->entity();
        if (!entity.has<sill::MeshComponent>()) continue;

        if (object->kind() == "pedestal") {
            auto& pedestal = object->as<Pedestal>();
            if (pedestal.substanceRevealNeeded() != substanceName) continue;

            object->as<Pedestal>().powered(ghostFactor == 0.f);
            for (auto& brickInfo : pedestal.brickInfos()) {
                entitySubstanceGhostFactor(brickInfo.brick->entity(), "", ghostFactor);
            }
            entitySubstanceGhostFactor(entity, "", ghostFactor); // @fixme The powered state should do that by itself!
        }
        else if (object->kind() == "panel") {
            auto& panel = object->as<Panel>();
            if (panel.substanceRevealNeeded() != substanceName) continue;

            object->as<Panel>().powered(ghostFactor == 0.f);
            entitySubstanceGhostFactor(entity, "", ghostFactor);
        }
    }
}
