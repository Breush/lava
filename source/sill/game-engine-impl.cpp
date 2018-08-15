#include "./game-engine-impl.hpp"

#include <chrono>
#include <lava/chamber/logger.hpp>
#include <lava/magma/material.hpp>

#include "./game-entity-impl.hpp"

using namespace lava::sill;
using namespace lava::chamber;

GameEngine::Impl::Impl(GameEngine& base)
    : m_fontManager(base)
{
    //----- Initializing window

    Extent2d windowExtent = {800, 600};
    m_window = std::make_unique<crater::Window>(windowExtent, "sill");

    //----- Initializing rendering

    // Register our materials
    m_renderEngine = std::make_unique<magma::RenderEngine>();
    registerMaterials();

    m_windowRenderTarget = &m_renderEngine->make<magma::WindowRenderTarget>(m_window->handle(), windowExtent);
    m_renderScene = &m_renderEngine->make<magma::RenderScene>();

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

void GameEngine::Impl::run()
{
    static const std::chrono::nanoseconds updateTime(11111111); // 1/60s * 2/3
    static auto currentTime = std::chrono::high_resolution_clock::now();
    static std::chrono::nanoseconds updateTimeLag(0);

    // Keep running while the window is open.
    while (m_window->opened()) {
        auto elapsedTime = std::chrono::high_resolution_clock::now() - currentTime;
        updateTimeLag += elapsedTime;
        currentTime += elapsedTime;

        // We play the game at a constant rate (updateTime).
        while (updateTimeLag >= updateTime) {
            // Treat all inputs since last frame.
            updateInput();

            // Update physics.
            m_physicsEngine->update(std::chrono::duration<float>(updateTime).count());

            // Update all entities.
            updateEntities();

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
    m_inputManager.updateReset();
    while (auto event = m_window->pollEvent()) {
        handleEvent(*event);

        m_inputManager.update(*event);
    }
}

void GameEngine::Impl::updateEntities()
{
    // Add all new components
    for (auto& entity : m_pendingAddedEntities) {
        m_entities.emplace_back(std::move(entity));
    }
    m_pendingAddedEntities.clear();

    // Indeed update entities
    for (auto& entity : m_entities) {
        entity->impl().update();
    }

    // @todo Remove pending removed entities
}

void GameEngine::Impl::registerMaterials()
{
    // @todo Could be moved to ashe via a nice API.
    // Which means a simple parser to be able not to write all that by ourselves.

    // Sky material
    m_renderEngine->registerMaterialFromFile("sky", "./data/shaders/materials/sky-material.simpl",
                                             {{"texture", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});

    // Font material
    m_renderEngine->registerMaterialFromFile("font", "./data/shaders/materials/font-material.simpl",
                                             {{"fontTexture", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});

    // Roughness-metallic material
    m_renderEngine->registerMaterialFromFile(
        "roughness-metallic", "./data/shaders/materials/rm-material.simpl",
        {{"albedoColor", magma::UniformType::VEC4, glm::vec4(1.f, 1.f, 1.f, 1.f)},
         {"roughnessFactor", magma::UniformType::FLOAT, 1.f},
         {"metallicFactor", magma::UniformType::FLOAT, 1.f},
         {"normalMap", magma::UniformType::TEXTURE, magma::UniformTextureType::NORMAL},
         {"albedoMap", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE},
         {"ormMap", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE},
         {"emissiveMap", magma::UniformType::TEXTURE, magma::UniformTextureType::INVISIBLE}});

    // Matcap material
    m_renderEngine->registerMaterialFromFile("matcap", "./data/shaders/materials/matcap-material.simpl",
                                             {{"texture", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});
}

void GameEngine::Impl::handleEvent(WsEvent& event)
{
    switch (event.type) {
    case WsEventType::WindowClosed: {
        m_window->close();
        break;
    }

    case WsEventType::KeyPressed: {
        if (event.key.which == Key::Escape) {
            m_window->close();
        }

        break;
    }

    case WsEventType::WindowResized: {
        // Ignore resize of same size
        Extent2d extent = {event.windowSize.width, event.windowSize.height};
        if (m_windowRenderTarget->extent() == extent) {
            break;
        }

        // @todo Might need to debounce that!
        // Or update swapchain
        m_windowRenderTarget->extent(extent);

        // @fixme This might cause an issue!
        // We should warn the camera component somehow.
        // Can be fixed while fixing #34.
        // m_camera->extent(extent);
        break;
    }

    default: break;
    }
}
