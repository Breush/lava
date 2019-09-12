#include <lava/sill/game-engine.hpp>

#include <lava/sill/game-entity.hpp>

#include "./game-engine-impl.hpp"

using namespace lava;
using namespace lava::sill;

$pimpl_class_forward(GameEngine);

$pimpl_method(GameEngine, InputManager&, input);
$pimpl_method(GameEngine, dike::PhysicsEngine&, physicsEngine);
$pimpl_method(GameEngine, magma::RenderEngine&, renderEngine);
$pimpl_method(GameEngine, magma::Scene&, scene);

// ----- Fonts
$pimpl_method(GameEngine, Font&, font, const std::string&, hrid);

// ----- Adders
void GameEngine::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    m_impl->add(std::move(gameEntity));
}

$pimpl_method(GameEngine, void, remove, const GameEntity&, entity);

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

    return pickedEntity;
}
