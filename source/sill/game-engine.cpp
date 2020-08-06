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

GameEngine::GameEngine()
{
    // ----- Rendering

    m_renderEngine = std::make_unique<magma::RenderEngine>();

    auto& scene = m_scenes[addScene()];
    scene->rendererType(magma::RendererType::Forward);
    scene->msaa(magma::Msaa::Max);

    if (m_renderEngine->vr().enabled()) {
        scene->msaa(magma::Msaa::None);
    }

    registerMaterialFromFile("font", "./data/shaders/materials/font-material.shmag"); // (used in TextMeshComponent)
    registerMaterialFromFile("ui.quad", "./data/shaders/flat-materials/ui/quad.shmag"); // (UI)

    // ----- Impl

    // @todo Might not be useful anymore, we have no back-end to hide
    m_impl = new GameEngine::Impl(*this);
}

GameEngine::~GameEngine()
{
    delete m_impl;
}

$pimpl_method(GameEngine, dike::PhysicsEngine&, physicsEngine);
$pimpl_method(GameEngine, magma::Scene&, scene2d);
$pimpl_method(GameEngine, magma::Camera&, camera2d);
$pimpl_method(GameEngine, magma::WindowRenderTarget&, windowRenderTarget);

uint8_t GameEngine::addScene()
{
    auto& scene = m_renderEngine->make<magma::Scene>();
    m_scenes.emplace_back(&scene);
    return m_scenes.size() - 1u;
}

$pimpl_method(GameEngine, crater::Window&, window);

$pimpl_property_v(GameEngine, bool, fpsCounting);

// ----- Adders
void GameEngine::add(std::unique_ptr<GameEntity>&& entity)
{
    m_entities.emplace_back(entity.get());

    m_impl->add(std::move(entity));
}

void GameEngine::remove(GameEntity& entity)
{
    auto entityIt = std::find(m_entities.begin(), m_entities.end(), &entity);
    m_entities.erase(entityIt);
    entity.alive(false);

    m_impl->remove(entity);
}

// ----- Materials

$pimpl_method(GameEngine, void, environmentTexture, const fs::Path&, imagesPath);

// @fixme This wrapper is almost useless...
void GameEngine::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath)
{
    m_renderEngine->registerMaterialFromFile(hrid, shaderPath);
}

$pimpl_method(GameEngine, void, run);

// ----- Tools

GameEntity* GameEngine::pickEntity(const Ray& ray, PickPrecision pickPrecision) const
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

// ----- Callbacks

uint32_t GameEngine::onWindowExtentChanged(WindowExtentChangedCallback&& callback)
{
    return m_impl->onWindowExtentChanged(std::move(callback));
}

$pimpl_method(GameEngine, void, removeOnWindowExtentChanged, uint32_t, id);
