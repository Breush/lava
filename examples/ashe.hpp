#pragma once

#include <iostream>
#include <lava/magma.hpp>
#include <memory>

namespace lava::ashe {
    class Application {
    public:
        Application(const std::string& title) { create(title); }

        magma::RenderEngine& engine() { return *m_engine; }
        magma::RenderWindow& window() { return *m_window; }
        magma::OrbitCamera& camera() { return *m_camera; }
        magma::PointLight& light() { return *m_light; }

        /**
         * Create base elements for the examples.
         */
        inline void create(const std::string& title)
        {
            // Render engine: the global manager.
            m_engine = std::make_unique<magma::RenderEngine>();

            // A window we can draw to.
            m_window = std::make_unique<magma::RenderWindow>(crater::VideoMode{800, 600}, title);
            m_engine->add(*m_window);

            // A camera.
            m_camera = &m_engine->make<magma::OrbitCamera>();
            m_camera->position({0.f, 2.f, 0.75f});
            m_camera->target({0.f, 0.f, 0.5f});
            m_camera->viewportRatio(800.f / 600.f);

            // A light.
            m_light = &m_engine->make<magma::PointLight>();
            m_light->position({5.f, 5.f, 0.f});
        }

        // @todo Allow custom eventHandler
        inline void run()
        {
            // Keep running while the window is open.
            while (m_window->opened()) {
                // Treat all events since last frame.
                crater::Event event;
                while (m_window->pollEvent(event)) {
                    handleEvent(event);
                }

                // Update the logic.
                m_engine->update();

                // Render the scene.
                m_engine->draw();
            }
        }

    protected:
        inline void handleEvent(crater::Event& event)
        {
            static auto buttonPressed = crater::input::Button::Unknown;
            static glm::vec2 lastDragPosition;

            switch (event.type) {
            case crater::Event::WindowClosed: {
                m_window->close();
                break;
            }

            case crater::Event::KeyPressed: {
                if (event.key.which == crater::input::Key::Escape) {
                    m_window->close();
                }
                // @todo Write better controls for the light
                else if (event.key.which == crater::input::Key::Right) {
                    m_light->position(m_light->position() - glm::vec3{0.1f, 0.f, 0.f});
                }
                else if (event.key.which == crater::input::Key::Left) {
                    m_light->position(m_light->position() + glm::vec3{0.1f, 0.f, 0.f});
                }
                else if (event.key.which == crater::input::Key::Up) {
                    m_light->position(m_light->position() - glm::vec3{0.f, 0.1f, 0.f});
                }
                else if (event.key.which == crater::input::Key::Down) {
                    m_light->position(m_light->position() + glm::vec3{0.f, 0.1f, 0.f});
                }
                break;
            }

            case crater::Event::WindowResized: {
                m_window->refresh();
                m_camera->viewportRatio(static_cast<float>(event.size.width) / static_cast<float>(event.size.height));
                break;
            }

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

    private:
        std::unique_ptr<magma::RenderEngine> m_engine = nullptr;
        std::unique_ptr<magma::RenderWindow> m_window = nullptr;
        magma::OrbitCamera* m_camera = nullptr;
        magma::PointLight* m_light = nullptr;
    };
}
