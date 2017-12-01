#include "./game-engine-impl.hpp"

#include <chrono>
#include <fstream>
#include <lava/magma/material.hpp>
#include <sstream>

#include "./game-entity-impl.hpp"

using namespace lava::sill;

GameEngine::Impl::Impl(GameEngine& base)
    : m_base(base)
    , m_fontManager(base)
{
    //----- Initializing rendering

    // Register our materials
    m_renderEngine = std::make_unique<magma::RenderEngine>();
    registerMaterials();

    m_renderWindow = &m_renderEngine->make<magma::RenderWindow>(crater::VideoMode{800, 600}, "sill");
    m_renderScene = &m_renderEngine->make<magma::RenderScene>();

    // @todo Handle custom cameras
    m_camera = &m_renderScene->make<magma::OrbitCamera>(Extent2d{800, 600});
    m_camera->position({2.f, 3.f, 2.f});
    m_camera->target({0.f, 0.f, 0.f});

    // @todo Handle custom lights
    m_light = &m_renderScene->make<magma::PointLight>();
    m_light->position({-1.f, -1.f, 0.7f});
    m_light->radius(100.f);

    // @todo Handle custom views
    m_renderEngine->addView(*m_camera, *m_renderWindow, Viewport{0, 0, 1, 1});

    //----- Initializing fonts

    m_fontManager.registerFont("default", "./assets/fonts/roboto-condensed_light.ttf");
}

void GameEngine::Impl::run()
{
    static const std::chrono::nanoseconds updateTime(11111111); // 1/60s * 2/3
    static auto currentTime = std::chrono::high_resolution_clock::now();
    static std::chrono::nanoseconds updateTimeLag(0);

    // Keep running while the window is open.
    while (m_renderWindow->opened()) {
        auto elapsedTime = std::chrono::high_resolution_clock::now() - currentTime;
        updateTimeLag += elapsedTime;
        currentTime += elapsedTime;

        // Treat all events since last frame.
        while (auto event = m_renderWindow->pollEvent()) {
            handleEvent(*event);
        }

        // We play the game at a constant rate (updateTime)
        while (updateTimeLag >= updateTime) {
            // Update all entities.
            for (auto& entity : m_entities) {
                entity->impl().update();
            }

            for (auto& entity : m_entities) {
                entity->impl().postUpdate();
            }

            updateTimeLag -= updateTime;
        }

        // Render the scene.
        m_renderEngine->draw();
    }
}

void GameEngine::Impl::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    m_entities.emplace_back(std::move(gameEntity));
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

void GameEngine::Impl::registerMaterials()
{
    // Sky material
    {
        std::ifstream fileStream("./data/shaders/materials/sky-material.simpl");
        std::stringstream buffer;
        buffer << fileStream.rdbuf();

        m_renderEngine->registerMaterial("sky", buffer.str(),
                                         {{"texture", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});
    }

    // Font material
    {
        std::ifstream fileStream("./data/shaders/materials/font-material.simpl");
        std::stringstream buffer;
        buffer << fileStream.rdbuf();

        m_renderEngine->registerMaterial("font", buffer.str(),
                                         {{"fontTexture", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});
    }

    // Roughness-metallic material
    {
        std::ifstream fileStream("./data/shaders/materials/rm-material.simpl");
        std::stringstream buffer;
        buffer << fileStream.rdbuf();

        m_renderEngine->registerMaterial("roughness-metallic", buffer.str(),
                                         {{"roughnessFactor", magma::UniformType::FLOAT, 1.f},
                                          {"metallicFactor", magma::UniformType::FLOAT, 1.f},
                                          {"normalMap", magma::UniformType::TEXTURE, magma::UniformTextureType::NORMAL},
                                          {"albedoMap", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE},
                                          {"ormMap", magma::UniformType::TEXTURE, magma::UniformTextureType::WHITE}});
    }
}

void GameEngine::Impl::handleEvent(crater::Event& event)
{
    static auto buttonPressed = crater::input::Button::Unknown;
    static glm::vec2 lastDragPosition;

    switch (event.type) {
    case crater::Event::WindowClosed: {
        m_renderWindow->close();
        break;
    }

    case crater::Event::KeyPressed: {
        if (event.key.which == crater::input::Key::Escape) {
            m_renderWindow->close();
        }

        break;
    }

    case crater::Event::WindowResized: {
        m_camera->extent({event.size.width, event.size.height});
        break;
    }

    // @todo Abstracted camera controls (make the OrbitCamera an Entity - and forward events)

    case crater::Event::MouseButtonPressed: {
        buttonPressed = event.mouseButton.which;
        lastDragPosition.x = event.mouseButton.x;
        lastDragPosition.y = event.mouseButton.y;
        break;
    }

    case crater::Event::MouseButtonReleased: {
        buttonPressed = crater::input::Button::Unknown;
        break;
    }

    case crater::Event::MouseScrolled: {
        m_camera->radiusAdd(-event.mouseScroll.delta / 10.f);
        break;
    }

    case crater::Event::MouseMoved: {
        if (buttonPressed == crater::input::Button::Unknown) return;

        glm::vec2 position(event.mouseMove.x, event.mouseMove.y);
        auto delta = (position - lastDragPosition) / 100.f;
        lastDragPosition = position;

        // Orbit with left button
        if (buttonPressed == crater::input::Button::Left) {
            m_camera->orbitAdd(-delta.x, -delta.y);
        }
        // Strafe with right button
        else if (buttonPressed == crater::input::Button::Right) {
            m_camera->strafe(delta.x / 10.f, delta.y / 10.f);
        }
        break;
    }

    default: break;
    }
}
