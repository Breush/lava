#pragma once

#include <fstream>
#include <lava/chamber.hpp>
#include <lava/crater.hpp>
#include <lava/magma.hpp>
#include <memory>
#include <sstream>

namespace lava::ashe {
    class Application {
        enum class Axis {
            X,
            Y,
            Z,
        };

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
            m_camera->translation({2.f, 2.f, 1.f});
            m_camera->target({0.f, 0.f, 0.5f});

            // A light.
            m_light = &m_scene->make<magma::PointLight>();
            m_light->translation({0.8f, 0.7f, 0.4f});
            m_light->radius(10.f);

            // We decide to show the scene's camera "0" at a certain translation in the window.
            m_engine->addView(*m_camera, *m_windowRenderTarget, Viewport{0, 0, 1, 1});

            // Gizmos.
            makeAxisGizmo(Axis::X, {1.f, 0.f, 0.f, 1.f});
            makeAxisGizmo(Axis::Y, {0.f, 1.f, 0.f, 1.f});
            makeAxisGizmo(Axis::Z, {0.f, 0.f, 1.f, 1.f});
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

        void makeAxisGizmo(Axis axis, glm::vec4 color)
        {
            auto& material = m_scene->make<magma::Material>("ashe");
            material.set("color", color);

            // The cylinder
            auto& cylinder = makeCylinder(0.01f, 1.f, axis);
            cylinder.material(material);
        }

        magma::Mesh& makeCylinder(float radius, float length, Axis axis)
        {
            const auto tessellation = 8u;
            auto& mesh = m_scene->make<magma::Mesh>();

            std::vector<glm::vec3> positions;
            positions.reserve(2u * tessellation);
            std::vector<uint16_t> indices;
            indices.reserve(6u * tessellation);

            // The circles
            const auto step = chamber::math::TWO_PI / tessellation;
            const auto cStep = chamber::math::cos(step);
            const auto sStep = chamber::math::sin(step);

            glm::vec2 point{radius, 0.f};
            for (auto j = 0u; j < tessellation; ++j) {
                if (axis == Axis::X) {
                    positions.emplace_back(glm::vec3{0, point.x, point.y});
                    positions.emplace_back(glm::vec3{length, point.x, point.y});
                }
                else if (axis == Axis::Y) {
                    positions.emplace_back(glm::vec3{point.x, 0, point.y});
                    positions.emplace_back(glm::vec3{point.x, length, point.y});
                }
                else if (axis == Axis::Z) {
                    positions.emplace_back(glm::vec3{point.x, point.y, 0});
                    positions.emplace_back(glm::vec3{point.x, point.y, length});
                }
                const auto px = point.x;
                point.x = px * cStep - point.y * sStep;
                point.y = px * sStep + point.y * cStep;
            }

            // The indices
            for (auto j = 0u; j < tessellation; ++j) {
                auto i0 = 2u * j;
                auto i1 = 2u * j + 1u;
                auto i2 = (2u * j + 2u) % (2u * tessellation);
                auto i3 = (2u * j + 3u) % (2u * tessellation);

                indices.emplace_back(i0);
                indices.emplace_back(i2);
                indices.emplace_back(i1);
                indices.emplace_back(i1);
                indices.emplace_back(i2);
                indices.emplace_back(i3);
            }

            mesh.verticesCount(positions.size());
            mesh.verticesPositions(positions);
            mesh.indices(indices);

            return mesh;
        }

        magma::Mesh& makePlane(glm::vec2 dimensions)
        {
            auto& mesh = m_scene->make<magma::Mesh>();

            std::vector<glm::vec3> positions(4);
            std::vector<glm::vec3> normals(4, {0.f, 0.f, 1.f});
            std::vector<glm::vec4> tangents(4, {1.f, 0.f, 0.f, 1.f});
            std::vector<uint16_t> indices = {0u, 1u, 2u, 2u, 3u, 0u};

            const auto halfWidth = dimensions.x / 2.f;
            const auto halfHeight = dimensions.y / 2.f;

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
            static uint32_t depthViewId = -1u;

            switch (event.type) {
            case WsEventType::WindowClosed: {
                m_window->close();
                break;
            }

            case WsEventType::KeyPressed: {
                glm::vec3 lightDelta;

                if (event.key.which == Key::Escape) {
                    m_window->close();
                }
                // Press C to show debug of camera
                else if (event.key.which == Key::C) {
                    if (depthViewId == -1u) {
                        depthViewId =
                            m_engine->addView(m_camera->depthRenderImage(), *m_windowRenderTarget, Viewport{0, 0, 1, 1});
                    }
                    else {
                        m_engine->removeView(depthViewId);
                        depthViewId = -1u;
                    }
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

                m_light->translation(m_light->translation() + lightDelta);
                break;
            }

            case WsEventType::WindowResized: {
                Extent2d extent{event.windowSize.width, event.windowSize.height};
                m_windowRenderTarget->extent(extent);
                m_camera->extent(extent);
                break;
            }

            case WsEventType::MouseButtonPressed: {
                buttonPressed = event.mouseButton.which;
                lastDragPosition.x = event.mouseButton.x;
                lastDragPosition.y = event.mouseButton.y;
                break;
            }

            case WsEventType::MouseButtonReleased: {
                buttonPressed = MouseButton::Unknown;
                break;
            }

            case WsEventType::MouseWheelScrolled: {
                if (event.mouseWheel.which != MouseWheel::Vertical) break;
                m_camera->radiusAdd(-event.mouseWheel.delta * m_camera->radius() / 10.f);
                break;
            }

            case WsEventType::MouseMoved: {
                if (buttonPressed == MouseButton::Unknown) return;

                glm::vec2 translation(event.mouseMove.x, event.mouseMove.y);
                auto delta = (translation - lastDragPosition) / 100.f;
                lastDragPosition = translation;

                // Orbit with left button
                if (buttonPressed == MouseButton::Left) {
                    m_camera->orbitAdd(-delta.x, delta.y);
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
