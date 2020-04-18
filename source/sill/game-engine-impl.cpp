#include "./game-engine-impl.hpp"

#include "./game-entity-impl.hpp"

using namespace lava::sill;
using namespace lava::chamber;

GameEngine::Impl::Impl(GameEngine& engine)
    : m_engine(engine)
{
    chamber::startProfiling();
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    //----- Initializing window

    m_windowExtent = {1600, 900};
    m_window = std::make_unique<crater::Window>(m_windowExtent, "sill");

    //----- Initializing rendering

    // Register our materials
    m_renderEngine = std::make_unique<magma::RenderEngine>();
    registerMaterials();

    m_windowRenderTarget = &m_renderEngine->make<magma::WindowRenderTarget>(m_window->handle(), m_windowExtent);

    m_scene = &m_renderEngine->make<magma::Scene>();
    m_scene->rendererType(magma::RendererType::Forward);
    m_scene->msaa(magma::Msaa::Max);

    if (m_renderEngine->vr().enabled()) {
        m_scene->msaa(magma::Msaa::None);
        m_vrRenderTarget = &m_renderEngine->make<magma::VrRenderTarget>();
        m_vrRenderTarget->bindScene(*m_scene);
    }

    // @todo Handle custom lights
    m_light = &m_scene->make<magma::Light>();
    m_lightController.bind(*m_light);
    m_lightController.direction({3.f, 2.f, -6.f});

    //----- Initializing 2D rendering

    m_scene2d = &m_renderEngine->make<magma::Scene>();
    m_scene2d->rendererType(magma::RendererType::ForwardFlat);

    m_camera2d = &m_scene2d->make<magma::Camera>(m_windowExtent);

    Viewport viewport;
    viewport.depth = -1.f; // On top.
    m_renderEngine->addView(m_camera2d->renderImage(), *m_windowRenderTarget, viewport);

    //----- Initializing physics

    m_physicsEngine = std::make_unique<dike::PhysicsEngine>();
    m_physicsEngine->gravity({0, 0, -10});

    //----- Initializing audio

    m_audioEngine = std::make_unique<flow::AudioEngine>();

    //----- Initializing fonts

    m_engine.font().registerFont("default", "./assets/fonts/roboto-condensed_light.ttf");
}

GameEngine::Impl::~Impl()
{
    m_destroying = true;

    chamber::stopProfiling();
}

void GameEngine::Impl::run()
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
            entity->impl().updateFrame();
        }

        // VR update
        m_engine.vr().update();

        // Render the scene.
        m_renderEngine->update();

        m_renderEngine->draw();
    }
}

void GameEngine::Impl::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    logger.info("sill.game-engine").tab(1) << "Adding entity " << gameEntity.get() << " '" << gameEntity->name() << "'." << std::endl;

    m_pendingAddedEntities.emplace_back(std::move(gameEntity));

    logger.log().tab(-1);
}

void GameEngine::Impl::remove(const GameEntity& gameEntity)
{
    logger.info("sill.game-engine").tab(1) << "Removing entity " << &gameEntity << " '" << gameEntity.name() << "'." << std::endl;

    m_pendingRemovedEntities.emplace_back(&gameEntity);

    logger.log().tab(-1);
}

void GameEngine::Impl::environmentTexture(const fs::Path& imagesPath)
{
    if (m_environmentTexture != nullptr) {
        // @todo We currently have no way to remove a texture,
        // but we should do that here.
    }

    m_environmentTexture = &m_scene->make<magma::Texture>();
    m_environmentTexture->loadCubeFromFiles(imagesPath);
    m_scene->environmentTexture(m_environmentTexture);
}

//----- Materials

void GameEngine::Impl::registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath)
{
    m_renderEngine->registerMaterialFromFile(hrid, shaderPath);
}

//----- Internals

void GameEngine::Impl::updateInput()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_engine.input().updateReset();

    while (auto event = m_window->pollEvent()) {
        bool propagate = true;
        handleEvent(*event, propagate);
        if (!propagate) continue;

        m_engine.input().update(*event);
    }

    // Handle VR event
    while (auto event = m_renderEngine->vr().pollEvent()) {
        m_engine.input().update(*event);
    }
}

void GameEngine::Impl::updateEntities(float dt)
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
        entity->impl().update(dt);
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

void GameEngine::Impl::registerMaterials()
{
    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    // Font material (used in TextMeshComponent)
    registerMaterialFromFile("font", "./data/shaders/materials/font-material.shmag");

    // PBR roughness-metallic material (used in GLB loader)
    registerMaterialFromFile("roughness-metallic", "./data/shaders/materials/rm-material.shmag");

    // UI
    registerMaterialFromFile("ui.quad", "./data/shaders/flat-materials/ui/quad.shmag");
}

void GameEngine::Impl::handleEvent(WsEvent& event, bool& propagate)
{
    // Let UI analyse the event.
    // @todo :Refactor Move the scene/camera2d to ui manager?
    m_engine.ui().handleEvent(event, propagate);

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
