#pragma once

#include <iostream>
#include <lava/magma.hpp>
#include <memory>

namespace lava::ashe {
    class Application {
    public:
        Application(const std::string& title) { create(title); }

        magma::RenderEngine& engine() { return *m_engine; }
        magma::RenderScene& scene() { return *m_scene; }
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
            m_engine->registerMaterial<magma::RmMaterial>();

            // A window we can draw to.
            m_window = &m_engine->make<magma::RenderWindow>(crater::VideoMode{800, 600}, title);

            // Render scene: holds what has to be drawn.
            m_scene = &m_engine->make<magma::RenderScene>();

            // A camera.
            m_camera = &m_scene->make<magma::OrbitCamera>(magma::Extent2d{800, 600});
            m_camera->position({0.f, 2.f, 0.75f});
            m_camera->target({0.f, 0.f, 0.5f});

            // A light.
            m_light = &m_scene->make<magma::PointLight>();
            m_light->position({0.8f, 0.7f, 0.4f});
            m_light->radius(10.f);

            m_lightSphere = &m_scene->make(magma::makers::sphereMeshMaker(12, 0.1f));
            m_lightSphere->positionAdd(m_light->position());

            // We decide to show the scene's camera "0" at a certain position in the window.
            m_engine->addView(*m_camera, *m_window, magma::Viewport{0, 0, 1, 1});
        }

        /// Simply run the main loop.
        inline void run()
        {
            run([](const crater::Event& /*event*/) {});
        }

        /// Running with a custom event handler.
        inline void run(std::function<void(const crater::Event&)> eventHandler)
        {
            // Keep running while the window is open.
            while (m_window->opened()) {
                // Treat all events since last frame.
                crater::Event event;
                while (m_window->pollEvent(event)) {
                    eventHandler(event);
                    handleEvent(event);
                }

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
                glm::vec3 lightDelta;

                if (event.key.which == crater::input::Key::Escape) {
                    m_window->close();
                }
                // @todo Write better controls for the light
                else if (event.key.which == crater::input::Key::Right) {
                    lightDelta = {-0.1f, 0.f, 0.f};
                }
                else if (event.key.which == crater::input::Key::Left) {
                    lightDelta = {0.1f, 0.f, 0.f};
                }
                else if (event.key.which == crater::input::Key::Up) {
                    lightDelta = {0.f, -0.1f, 0.f};
                }
                else if (event.key.which == crater::input::Key::Down) {
                    lightDelta = {0.f, 0.1f, 0.f};
                }

                m_light->position(m_light->position() + lightDelta);
                m_lightSphere->positionAdd(lightDelta);

                break;
            }

            case crater::Event::WindowResized: {
                m_camera->extent({event.size.width, event.size.height});
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
        magma::RenderScene* m_scene = nullptr;
        magma::RenderWindow* m_window = nullptr;
        magma::OrbitCamera* m_camera = nullptr;
        magma::PointLight* m_light = nullptr;
        magma::Mesh* m_lightSphere = nullptr;
    };
}
