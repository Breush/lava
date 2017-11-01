#include <lava/sill/game-engine.hpp>

#include <lava/chamber/macros.hpp>

#include "./game-engine-impl.hpp"

using namespace lava::sill;

$pimpl_class(GameEngine);

$pimpl_method(GameEngine, void, run);

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
