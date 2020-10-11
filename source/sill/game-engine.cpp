#include <lava/sill/game-engine.hpp>

#include <lava/sill/components/mesh-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/entity-frame.hpp>
#include <lava/sill/makers.hpp>

using namespace lava::sill;
using namespace lava::chamber;

namespace {
    static Entity* g_debugEntityPickingEntity = nullptr;
}

GameEngine::GameEngine()
{
    startProfiling();
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    //----- Windowing

    m_windowExtent = {1600, 900};
    m_window = std::make_unique<crater::Window>(m_windowExtent, "sill");

    //----- Rendering

    m_renderEngine = std::make_unique<magma::RenderEngine>();
    m_windowRenderTarget = &m_renderEngine->make<magma::WindowRenderTarget>(m_window->handle(), m_windowExtent);

    auto& scene = m_scenes[addScene()];
    scene->rendererType(magma::RendererType::Forward);
    scene->msaa(magma::Msaa::Max);

    if (m_renderEngine->vr().enabled()) {
        scene->msaa(magma::Msaa::None);
        m_vrRenderTarget = &m_renderEngine->make<magma::VrRenderTarget>();
        m_vrRenderTarget->bindScene(*scene);
    }

    // @todo Handle custom lights
    m_light = &scene->make<magma::Light>();
    m_lightController.bind(*m_light);
    m_lightController.direction({3.f, 2.f, -6.f});

    registerMaterialFromFile("font", "./data/shaders/materials/font-material.shmag"); // (TextMeshComponent)
    registerMaterialFromFile("ui.quad", "./data/shaders/flat-materials/ui/quad.shmag"); // (UI)

    //----- 2D rendering

    // @fixme Why don't we use addScene here?
    m_scene2d = &m_renderEngine->make<magma::Scene>();
    m_scene2d->rendererType(magma::RendererType::ForwardFlat);

    m_camera2d = &m_scene2d->make<magma::Camera>(m_windowExtent);

    Viewport viewport;
    viewport.depth = -1.f; // On top.
    m_renderEngine->addView(m_camera2d->renderImage(), *m_windowRenderTarget, viewport);

    //----- Physics

    m_physicsEngine = std::make_unique<dike::PhysicsEngine>();
    m_physicsEngine->gravity({0, 0, -10});

    //----- Audio

    m_audioEngine = std::make_unique<flow::AudioEngine>();

    //----- Fonts

    m_fontManager.registerFont("default", "./assets/fonts/noto_sans-condensed_light.ttf");
    m_fontManager.registerFont("default", "./assets/fonts/symbola.ttf");
}

GameEngine::~GameEngine()
{
    m_destroying = true;

    stopProfiling();
}

void GameEngine::run()
{
    PROFILE_FUNCTION();

    // @todo If we want two GameEngine at the same time on day,
    // these statics cannot stay there.
    static const std::chrono::nanoseconds updateTime(7'407'407); // 1/90s * 2/3
    static float updateDt = std::chrono::duration<float>(updateTime).count();
    static auto currentTime = std::chrono::high_resolution_clock::now();
    static std::chrono::nanoseconds updateTimeLag(updateTime);

    // Keep running while the window is open.
    while (m_window->opened()) {
        auto elapsedTime = std::chrono::high_resolution_clock::now() - currentTime;
        updateTimeLag += elapsedTime;
        currentTime += elapsedTime;

        if (m_fpsCounting) {
            m_fpsCount += 1u;
            m_fpsElapsedTime += elapsedTime;

            if (m_fpsElapsedTime > std::chrono::nanoseconds(1'000'000'000)) {
                logger.info("sill.game-engine")
                    << std::setprecision(3) << std::chrono::duration<float>(elapsedTime).count() * 1000.f << "ms " << m_fpsCount
                    << "FPS" << std::endl;
                m_fpsCount = 0u;
                m_fpsElapsedTime = std::chrono::nanoseconds(0);
            }
        }

        // We play the game at a constant rate (updateTime).
        while (updateTimeLag >= updateTime) {
            // Treat all inputs since last frame.
            updateInput();

            // Update physics.
            m_physicsEngine->update(updateDt);

            // Update audio.
            m_audioEngine->update(updateDt);

            // Update all entities.
            updateEntities(updateDt);

            updateTimeLag -= updateTime;
        }

        // Entities update before each rendering
        for (auto& entity : m_entities) {
            entity->updateFrame();
        }

        // VR update
        m_vrManager.update();

        // Render the scene.
        m_renderEngine->update();
        m_renderEngine->draw();
    }
}

// ----- Rendering

uint8_t GameEngine::addScene()
{
    auto& scene = m_renderEngine->make<magma::Scene>();
    m_scenes.emplace_back(&scene);
    return m_scenes.size() - 1u;
}

// ----- Adders

void GameEngine::add(std::unique_ptr<Entity>&& entity)
{
    logger.info("sill.game-engine").tab(1) << "Adding entity " << entity.get() << " '" << entity->name() << "'." << std::endl;

    m_allEntities.emplace_back(entity.get());
    m_pendingAddedEntities.emplace_back(std::move(entity));

    logger.log().tab(-1);
}

void GameEngine::add(std::unique_ptr<EntityFrame>&& entityFrame)
{
    // @note A priori, we don't need a pending added list,
    // as we do not iterate over entityFrames, just store them.
    m_entityFrames.emplace_back(std::move(entityFrame));
}

void GameEngine::remove(Entity& entity)
{
    logger.info("sill.game-engine").tab(1) << "Removing entity " << &entity << " '" << entity.name() << "'." << std::endl;

    auto entityIt = std::find(m_allEntities.begin(), m_allEntities.end(), &entity);
    m_allEntities.erase(entityIt);
    entity.warnRemoved();

    m_pendingRemovedEntities.emplace_back(&entity);

    logger.log().tab(-1);
}

void GameEngine::remove(EntityFrame& entityFrame)
{
    auto entityFrameIt = std::find_if(m_entityFrames.begin(), m_entityFrames.end(), [&entityFrame](const std::unique_ptr<EntityFrame>& entityFramePtr) {
        return entityFramePtr.get() == &entityFrame;
    });
    m_entityFrames.erase(entityFrameIt);
    entityFrame.warnRemoved();
}

// ----- Materials

void GameEngine::environmentTexture(const fs::Path& imagesPath, uint8_t sceneIndex)
{
    auto environmentTexture = m_renderEngine->makeTexture();
    environmentTexture->loadCubeFromFiles(imagesPath);
    m_scenes[sceneIndex]->environmentTexture(environmentTexture);
}

// @fixme This wrapper is useless, and the one above kind of too...
void GameEngine::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath)
{
    m_renderEngine->registerMaterialFromFile(hrid, shaderPath);
}

// ----- Callbacks

uint32_t GameEngine::onWindowExtentChanged(WindowExtentChangedCallback&& callback)
{
    auto id = m_windowExtentChangedNextId;
    m_windowExtentChangedNextId += 1u;
    m_windowExtentChangedCallbacks[id] = std::move(callback);
    return id;
}

void GameEngine::removeOnWindowExtentChanged(uint32_t id) {
    if (m_destroying) return;
    m_windowExtentChangedCallbacks.erase(id);
}

// ----- Tools

Entity* GameEngine::pickEntity(const Ray& ray, PickPrecision pickPrecision, float* distance) const
{
    Entity* pickedEntity = nullptr;

    float minDistance = INFINITY;
    for (const auto& entity : m_entities) {
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

    if (distance != nullptr) {
        *distance = minDistance;
    }

    return pickedEntity;
}

void GameEngine::debugEntityPicking(bool debugEntityPicking)
{
    if (m_debugEntityPicking == debugEntityPicking) return;
    m_debugEntityPicking = debugEntityPicking;

    if (m_debugEntityPicking) {
        g_debugEntityPickingEntity = &make<Entity>("debug.entity-picking");
        makers::sphereMeshMaker(32u, 0.05f)(g_debugEntityPickingEntity->ensure<MeshComponent>());
        g_debugEntityPickingEntity->get<TransformComponent>().scaling(0.f);
    }
    else {
        remove(*g_debugEntityPickingEntity);
        g_debugEntityPickingEntity = nullptr;
    }
}

Entity* GameEngine::findEntityByName(const std::string& name) const
{
    for (auto entity : m_allEntities) {
        if (entity->name() == name) {
            return entity;
        }
    }
    return nullptr;
}

//----- Internals

void GameEngine::updateInput()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_inputManager.updateReset();

    while (auto event = m_window->pollEvent()) {
        bool propagate = true;
        handleEvent(*event, propagate);
        if (!propagate) continue;

        m_inputManager.update(*event);
    }

    // Handle VR event
    auto& vr = m_renderEngine->vr();
    while (auto event = vr.pollEvent()) {
        m_inputManager.update(*event);
    }
}

void GameEngine::updateEntities(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Update based on callbacks
    if (m_windowExtentDelay > 0.f) {
        m_windowExtentDelay -= dt;
        if (m_windowExtentDelay <= 0.f) {
            m_windowRenderTarget->extent(m_windowExtent);
            m_camera2d->extent(m_windowExtent);
            for (const auto& callbackPair : m_windowExtentChangedCallbacks) {
                callbackPair.second(m_windowExtent);
            }
        }
    }

    // Add all new entities
    for (auto& entity : m_pendingAddedEntities) {
        m_entities.emplace_back(std::move(entity));
    }
    m_pendingAddedEntities.clear();

    // Indeed update entities
    for (auto& entity : m_entities) {
        if (!entity->active()) continue;
        entity->update(dt);
    }

    // Remove pending entities
    if (!m_pendingRemovedEntities.empty()) {
        // Some entities might remove others, so we copy everything before-hand.
        auto pendingRemovedEntities = m_pendingRemovedEntities;
        m_pendingRemovedEntities.clear();

        for (auto pEntity : pendingRemovedEntities) {
            for (auto iEntity = m_entities.begin(); iEntity != m_entities.end();) {
                if (iEntity->get() == pEntity) {
                    iEntity = m_entities.erase(iEntity);
                    continue;
                }
                iEntity++;
            }
        }
    }
}

void GameEngine::handleEvent(WsEvent& event, bool& propagate)
{
    // Let UI analyse the event.
    // @todo :Refactor Move the scene/camera2d to ui manager?
    m_uiManager.handleEvent(event, propagate);

    switch (event.type) {
    case WsEventType::WindowClosed: {
        m_window->close();
        break;
    }

    case WsEventType::WindowResized: {
        // Ignore resize of same size
        Extent2d extent = {event.windowSize.width, event.windowSize.height};
        if (m_windowExtent == extent) {
            break;
        }

        // Or update swapchain
        m_windowExtent = extent;
        m_windowExtentDelay = 0.1f;
        break;
    }

    default: break;
    }
}
