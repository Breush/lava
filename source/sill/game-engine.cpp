#include <lava/sill/game-engine.hpp>

#include "./game-engine-impl.hpp"

using namespace lava;
using namespace lava::sill;

$pimpl_class_forward(GameEngine);

$pimpl_method(GameEngine, InputManager&, input);
$pimpl_method(GameEngine, dike::PhysicsEngine&, physicsEngine);

// ----- Fonts
$pimpl_method(GameEngine, Font&, font, const std::string&, hrid);

// ----- Adders
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

// ----- Materials
$pimpl_method(GameEngine, void, registerMaterialFromFile, const std::string&, hrid, const fs::Path&, shaderPath);

// ----- VR
$pimpl_method_const(GameEngine, bool, vrEnabled);
$pimpl_method_const(GameEngine, bool, vrDeviceValid, VrDeviceType, deviceType);
$pimpl_method_const(GameEngine, const glm::mat4&, vrDeviceTransform, VrDeviceType, deviceType);

$pimpl_method(GameEngine, void, run);
