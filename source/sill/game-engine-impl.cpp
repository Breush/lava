#include "./game-engine-impl.hpp"

#include <lava/magma/materials/rm-material.hpp>

#include "./game-entity-impl.hpp"

using namespace lava::sill;

GameEngine::Impl::Impl()
{
    //----- Initializing rendering

    // Register our materials
    m_renderEngine = std::make_unique<magma::RenderEngine>();
    m_renderEngine->registerMaterial<magma::RmMaterial>();

    m_renderWindow = &m_renderEngine->make<magma::RenderWindow>(crater::VideoMode{800, 600}, "sill");
    m_renderScene = &m_renderEngine->make<magma::RenderScene>();

    // @todo Handle custom cameras
    m_camera = &m_renderScene->make<magma::OrbitCamera>(Extent2d{800, 600});
    m_camera->position({0.f, 2.f, 0.75f});
    m_camera->target({0.f, 0.f, 0.5f});

    // @todo Handle custom lights
    m_light = &m_renderScene->make<magma::PointLight>();
    m_light->position({0.8f, 0.7f, 0.4f});
    m_light->radius(10.f);

    // @todo Handle custom views
    m_renderEngine->addView(*m_camera, *m_renderWindow, Viewport{0, 0, 1, 1});
}

void GameEngine::Impl::run()
{
    // Keep running while the window is open.
    while (m_renderWindow->opened()) {
        // Treat all events since last frame.
        crater::Event event;
        while (m_renderWindow->pollEvent(event)) {
            handleEvent(event);
        }

        // Update all entities.
        for (auto& entity : m_entities) {
            entity->impl().update();
        }

        for (auto& entity : m_entities) {
            entity->impl().postUpdate();
        }

        // Render the scene.
        m_renderEngine->draw();
    }
}

void GameEngine::Impl::add(std::unique_ptr<GameEntity>&& gameEntity)
{
    m_entities.emplace_back(std::move(gameEntity));
}

//----- Internals

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
