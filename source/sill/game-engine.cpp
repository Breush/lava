#include <lava/sill/game-engine.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/makers.hpp>

#include "./game-engine-impl.hpp"

using namespace lava;
using namespace lava::sill;

namespace {
    static GameEntity* g_debugEntityPickingEntity = nullptr;
}

$pimpl_class_forward(GameEngine);

$pimpl_method(GameEngine, InputManager&, input);
$pimpl_method(GameEngine, dike::PhysicsEngine&, physicsEngine);
$pimpl_method(GameEngine, magma::RenderEngine&, renderEngine);
$pimpl_method(GameEngine, magma::Scene&, scene);
$pimpl_method(GameEngine, magma::Scene&, scene2d);
$pimpl_method(GameEngine, magma::Camera&, camera2d);
$pimpl_method(GameEngine, magma::WindowRenderTarget&, windowRenderTarget);
$pimpl_method(GameEngine, crater::Window&, window);

$pimpl_property_v(GameEngine, bool, fpsCounting);

// ----- Fonts
$pimpl_method(GameEngine, Font&, font, const std::string&, hrid);

// ----- Adders
void GameEngine::add(std::unique_ptr<GameEntity>&& entity)
{
    m_entities.emplace_back(entity.get());

    m_impl->add(std::move(entity));
}

void GameEngine::remove(const GameEntity& entity)
{
    m_entities.erase(std::find(m_entities.begin(), m_entities.end(), &entity));

    m_impl->remove(entity);
}

// ----- Materials
$pimpl_method(GameEngine, void, environmentTexture, const fs::Path&, imagesPath);
$pimpl_method(GameEngine, void, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath);

$pimpl_method(GameEngine, void, run);

// ----- Tools

GameEntity* GameEngine::pickEntity(Ray ray, PickPrecision pickPrecision) const
{
    GameEntity* pickedEntity = nullptr;

    float minDistance = INFINITY;
    const auto& entities = m_impl->entities();
    for (const auto& entity : entities) {
        auto distance = entity->distanceFrom(ray, pickPrecision);
        if (distance > 0.f && distance < minDistance) {
            minDistance = distance;
            pickedEntity = entity.get();
        }
    }

    // Move a ball where we picked.
    if (m_debugEntityPicking) {
        if (pickedEntity) {
            g_debugEntityPickingEntity->get<TransformComponent>().scaling(1.f);
            g_debugEntityPickingEntity->get<TransformComponent>().translation(ray.origin + minDistance * ray.direction);
        }
        else {
            g_debugEntityPickingEntity->get<TransformComponent>().scaling(0.f);
        }
    }

    return pickedEntity;
}

void GameEngine::debugEntityPicking(bool debugEntityPicking)
{
    if (m_debugEntityPicking == debugEntityPicking) return;
    m_debugEntityPicking = debugEntityPicking;

    if (m_debugEntityPicking) {
        g_debugEntityPickingEntity = &make<GameEntity>("debug.entity-picking");
        makers::sphereMeshMaker(32u, 0.05f)(g_debugEntityPickingEntity->ensure<MeshComponent>());
        g_debugEntityPickingEntity->get<TransformComponent>().scaling(0.f);
    }
    else {
        remove(*g_debugEntityPickingEntity);
        g_debugEntityPickingEntity = nullptr;
    }
}

GameEntity* GameEngine::findEntityByName(const std::string& name) const
{
    for (auto entity : m_entities) {
        if (entity->name() == name) {
            return entity;
        }
    }
    return nullptr;
}
