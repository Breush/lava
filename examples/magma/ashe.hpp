#pragma once

#include <fstream>
#include <lava/crater.hpp>
#include <lava/magma.hpp>
#include <memory>
#include <sstream>

namespace lava::ashe {
    class Application {
    public:
        Application(const std::string& title) { create(title); }

        crater::Window& window() { return *m_window; }
        magma::RenderEngine& engine() { return *m_engine; }
        magma::WindowRenderTarget& windowRenderTarget() { return *m_windowRenderTarget; }
        magma::RenderScene& scene() { return *m_scene; }
        magma::OrbitCamera& camera() { return *m_camera; }
        magma::PointLight& light() { return *m_light; }

        /**
         * Create base elements for the examples.
         */
        inline void create(const std::string& title)
        {
            // Render engine: the global manager.
            m_engine = std::make_unique<magma::RenderEngine>();

            // Custom material
            std::ifstream fileStream("./examples/magma/ashe-material.simpl");
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            m_engine->registerMaterial("ashe", buffer.str(), {{"color", magma::UniformType::VEC4, glm::vec4(1.f)}});

            // A window we can draw to.
            m_window = std::make_unique<crater::Window>(Extent2d{800, 600}, title);
            m_windowRenderTarget = &m_engine->make<magma::WindowRenderTarget>(m_window->handle(), m_window->extent());

            // Render scene: holds what has to be drawn.
            m_scene = &m_engine->make<magma::RenderScene>();

            // A camera.
            m_camera = &m_scene->make<magma::OrbitCamera>(m_window->extent());
            m_camera->position({0.f, 2.f, 0.75f});
            m_camera->target({0.f, 0.f, 0.5f});

            // A light.
            m_light = &m_scene->make<magma::PointLight>();
            m_light->position({0.8f, 0.7f, 0.4f});
            m_light->radius(10.f);

            // We decide to show the scene's camera "0" at a certain position in the window.
            m_engine->addView(*m_camera, *m_windowRenderTarget, Viewport{0, 0, 1, 1});
        }

        /// Simply run the main loop.
        inline void run()
        {
            run([](const WsEvent& /*event*/) {});
        }

        /// Running with a custom event handler.
        inline void run(std::function<void(const WsEvent&)> eventHandler)
        {
            // Keep running while the window is open.
            while (m_window->opened()) {
                // Treat all events since last frame.
                while (auto event = m_window->pollEvent()) {
                    eventHandler(*event);
                    handleEvent(*event);
                }

                // Render the scene.
                m_engine->update();
                m_engine->draw();
            }
        }

        magma::Mesh& makePlane(Extent2d dimensions)
        {
            auto& mesh = m_scene->make<magma::Mesh>();

            std::vector<glm::vec3> positions(4);
            std::vector<glm::vec3> normals(4, {0.f, 0.f, 1.f});
            std::vector<glm::vec4> tangents(4, {1.f, 0.f, 0.f, 1.f});
            std::vector<uint16_t> indices = {0u, 1u, 2u, 2u, 3u, 0u};

            const auto halfWidth = dimensions.width / 2.f;
            const auto halfHeight = dimensions.height / 2.f;

            positions[0].x = -halfWidth;
            positions[0].y = -halfHeight;
            positions[1].x = halfWidth;
            positions[1].y = -halfHeight;
            positions[2].x = halfWidth;
            positions[2].y = halfHeight;
            positions[3].x = -halfWidth;
            positions[3].y = halfHeight;

            mesh.verticesCount(positions.size());
            mesh.verticesPositions(positions);
            mesh.verticesNormals(normals);
            mesh.verticesTangents(tangents);
            mesh.indices(indices);

            return mesh;
        }

    protected:
        inline void handleEvent(WsEvent& event)
        {
            static auto buttonPressed = MouseButton::Unknown;
            static glm::vec2 lastDragPosition;

            switch (event.type) {
            case WsEvent::WindowClosed: {
                m_window->close();
                break;
            }

            case WsEvent::KeyPressed: {
                glm::vec3 lightDelta;

                if (event.key.which == Key::Escape) {
                    m_window->close();
                }
                // @todo Write better controls for the light
                else if (event.key.which == Key::Right) {
                    lightDelta = {-0.1f, 0.f, 0.f};
                }
                else if (event.key.which == Key::Left) {
                    lightDelta = {0.1f, 0.f, 0.f};
                }
                else if (event.key.which == Key::Up) {
                    lightDelta = {0.f, -0.1f, 0.f};
                }
                else if (event.key.which == Key::Down) {
                    lightDelta = {0.f, 0.1f, 0.f};
                }

                m_light->position(m_light->position() + lightDelta);
                break;
            }

            case WsEvent::WindowResized: {
                Extent2d extent{event.windowSize.width, event.windowSize.height};
                m_windowRenderTarget->extent(extent);
                m_camera->extent(extent);
                break;
            }

            case WsEvent::MouseButtonPressed: {
                buttonPressed = event.mouseButton.which;
                lastDragPosition.x = event.mouseButton.x;
                lastDragPosition.y = event.mouseButton.y;
                break;
            }

            case WsEvent::MouseButtonReleased: {
                buttonPressed = MouseButton::Unknown;
                break;
            }

            case WsEvent::MouseScrolled: {
                m_camera->radiusAdd(-event.mouseScroll.delta / 10.f);
                break;
            }

            case WsEvent::MouseMoved: {
                if (buttonPressed == MouseButton::Unknown) return;

                glm::vec2 position(event.mouseMove.x, event.mouseMove.y);
                auto delta = (position - lastDragPosition) / 100.f;
                lastDragPosition = position;

                // Orbit with left button
                if (buttonPressed == MouseButton::Left) {
                    m_camera->orbitAdd(-delta.x, -delta.y);
                }
                // Strafe with right button
                else if (buttonPressed == MouseButton::Right) {
                    m_camera->strafe(delta.x / 10.f, delta.y / 10.f);
                }
                break;
            }

            default: break;
            }
        }

    private:
        std::unique_ptr<crater::Window> m_window = nullptr;
        std::unique_ptr<magma::RenderEngine> m_engine = nullptr;
        magma::WindowRenderTarget* m_windowRenderTarget = nullptr;
        magma::RenderScene* m_scene = nullptr;
        magma::OrbitCamera* m_camera = nullptr;
        magma::PointLight* m_light = nullptr;
    };
}
