#include <lava/sill/game-engine.hpp>

#include <lava/core/macros.hpp>

#include "./game-engine-impl.hpp"

using namespace lava;
using namespace lava::sill;

$pimpl_class_forward(GameEngine);

$pimpl_method(GameEngine, InputManager&, input);
$pimpl_method(GameEngine, dike::PhysicsEngine&, physicsEngine);

// ----- Fonts
$pimpl_method(GameEngine, Font&, font, const std::string&, hrid);

void GameEngine::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    m_impl->add(std::move(gameEntity));
}

void GameEngine::add(std::unique_ptr<Material>&& material)
{
    m_impl->add(std::move(material));
}

void GameEngine::add(std::unique_ptr<Texture>&& texture)
{
    m_impl->add(std::move(texture));
}

$pimpl_method(GameEngine, void, run);
