#include "./game-engine-impl.hpp"

#include "./game-entity-impl.hpp"

using namespace lava::sill;
using namespace lava::chamber;

GameEngine::Impl::Impl(GameEngine& base)
    : m_fontManager(base)
{
    chamber::startProfiling();
    PROFILE_FUNCTION(PROFILER_COLOR_INIT);

    //----- Initializing window

    Extent2d windowExtent = {800, 600};
    m_window = std::make_unique<crater::Window>(windowExtent, "sill");

    //----- Initializing rendering

    // Register our materials
    m_renderEngine = std::make_unique<magma::RenderEngine>();
    registerMaterials();

    m_windowRenderTarget = &m_renderEngine->make<magma::WindowRenderTarget>(m_window->handle(), windowExtent);

    m_renderScene = &m_renderEngine->make<magma::RenderScene>();
    m_renderScene->rendererType(magma::RendererType::Forward);

    if (m_renderEngine->vrEnabled()) {
        m_vrRenderTarget = &m_renderEngine->make<magma::VrRenderTarget>();
        m_vrRenderTarget->bindScene(*m_renderScene);
    }

    // @todo Handle custom lights
    m_light = &m_renderScene->make<magma::DirectionalLight>();
    m_light->translation({-5.f, -5.f, 5.f});
    m_light->direction({3.f, 2.f, -6.f});

    //----- Initializing physics

    m_physicsEngine = std::make_unique<dike::PhysicsEngine>();
    m_physicsEngine->gravity({0, 0, -10});

    //----- Initializing fonts

    m_fontManager.registerFont("default", "./assets/fonts/roboto-condensed_light.ttf");
}

GameEngine::Impl::~Impl()
{
    chamber::stopProfiling();
}

void GameEngine::Impl::run()
{
    PROFILE_FUNCTION();

    // @todo If we want two GameEngine at the same time on day,
    // these statics cannot stay there.
    static const std::chrono::nanoseconds updateTime(11'111'111); // 1/60s * 2/3
    static float updateDt = std::chrono::duration<float>(updateTime).count();
    static auto currentTime = std::chrono::high_resolution_clock::now();
    static std::chrono::nanoseconds updateTimeLag(updateTime);

    // Keep running while the window is open.
    while (m_window->opened()) {
        auto elapsedTime = std::chrono::high_resolution_clock::now() - currentTime;
        updateTimeLag += elapsedTime;
        currentTime += elapsedTime;

        if (m_fpsCounterEnabled) {
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

            // Update all entities.
            updateEntities(updateDt);

            updateTimeLag -= updateTime;
        }

        // Render the scene.
        m_renderEngine->update();
        m_renderEngine->draw();
    }
}

void GameEngine::Impl::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    logger.info("sill.game-engine") << "Adding entity " << &gameEntity->impl() << "." << std::endl;

    m_pendingAddedEntities.emplace_back(std::move(gameEntity));
}

void GameEngine::Impl::add(std::unique_ptr<Material>&& material)
{
    m_materials.emplace_back(std::move(material));
}

void GameEngine::Impl::add(std::unique_ptr<Texture>&& texture)
{
    m_textures.emplace_back(std::move(texture));
}

//----- Internals

void GameEngine::Impl::updateInput()
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    m_inputManager.updateReset();

    while (auto event = m_window->pollEvent()) {
        handleEvent(*event);

        m_inputManager.update(*event);
    }

    // Update VR controllers
    if (m_vrRenderTarget != nullptr) {
        if (m_renderEngine->vrDeviceValid(VrDeviceType::LeftHand)) {
            auto& mesh = m_renderEngine->vrDeviceMesh(VrDeviceType::LeftHand, *m_renderScene);
            mesh.transform(m_renderEngine->vrDeviceTransform(VrDeviceType::LeftHand));
        }
        if (m_renderEngine->vrDeviceValid(VrDeviceType::RightHand)) {
            auto& mesh = m_renderEngine->vrDeviceMesh(VrDeviceType::RightHand, *m_renderScene);
            mesh.transform(m_renderEngine->vrDeviceTransform(VrDeviceType::RightHand));
        }
        if (m_renderEngine->vrDeviceValid(VrDeviceType::Head)) {
            auto& mesh = m_renderEngine->vrDeviceMesh(VrDeviceType::Head, *m_renderScene);
            mesh.transform(m_renderEngine->vrDeviceTransform(VrDeviceType::Head));
        }
    }

    // Handle VR event
    while (auto event = m_renderEngine->vrPollEvent()) {
        m_inputManager.update(*event);
    }
}

void GameEngine::Impl::updateEntities(float dt)
{
    PROFILE_FUNCTION(PROFILER_COLOR_UPDATE);

    // Add all new components
    for (auto& entity : m_pendingAddedEntities) {
        m_entities.emplace_back(std::move(entity));
    }
    m_pendingAddedEntities.clear();

    // Indeed update entities
    for (auto& entity : m_entities) {
        entity->impl().update(dt);
    }

    // @todo Remove pending removed entities
}

void GameEngine::Impl::registerMaterials()
{
    PROFILE_FUNCTION(PROFILER_COLOR_REGISTER);

    // Font material (used in TextMeshComponent)
    m_renderEngine->registerMaterialFromFile("font", "./data/shaders/materials/font-material.shmag");

    // Roughness-metallic material (used in GLB loader)
    m_renderEngine->registerMaterialFromFile("roughness-metallic", "./data/shaders/materials/rm-material.shmag");

    // @fixme Can be moved to ashe, since there is no need to know magma uniform thingy.

    // Sky material
    m_renderEngine->registerMaterialFromFile("sky", "./data/shaders/materials/sky-material.shmag");

    // Matcap material
    m_renderEngine->registerMaterialFromFile("matcap", "./data/shaders/materials/matcap-material.shmag");
}

void GameEngine::Impl::handleEvent(WsEvent& event)
{
    PROFILE_FUNCTION();

    switch (event.type) {
    case WsEventType::WindowClosed: {
        m_window->close();
        break;
    }

    case WsEventType::KeyPressed: {
        if (event.key.which == Key::Escape) {
            m_window->close();
        }
        else if (event.key.which == Key::F11) {
            m_window->fullscreen(!m_window->fullscreen());
        }
        else if (event.key.which == Key::F) {
            // @todo Move that to ashe.
            m_renderEngine->logTrackingOnce();
            m_fpsCounterEnabled = true;
        }

        break;
    }

    case WsEventType::WindowResized: {
        // Ignore resize of same size
        Extent2d extent = {event.windowSize.width, event.windowSize.height};
        if (m_windowRenderTarget->extent() == extent) {
            break;
        }

        // Or update swapchain
        m_windowRenderTarget->extent(extent);

        for (const auto& callback : m_windowExtentChangedCallbacks) {
            callback(extent);
        }
        break;
    }

    default: break;
    }
}
