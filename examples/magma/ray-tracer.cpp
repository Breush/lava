/**
 * Shows how to use the ray-tracer.
 */

#include <lava/crater.hpp>
#include <lava/magma.hpp>

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    magma::RenderEngine engine;

    crater::Window window(crater::VideoMode{800, 600}, "ashe - magma | Ray tracer");
    auto& renderTarget = engine.make<magma::WindowRenderTarget>(window.handle(), window.extent());

    // Scene setup
    auto& scene = engine.make<magma::Scene>();
    scene.rendererType(magma::RendererType::RayTracer);

    // @fixme Stolen from shadows
    const auto minScaling = 0.3;
    const auto maxScaling = 0.5;

    // Structure
    for (auto i = 0u; i < 5u; ++i) {
        auto& mesh = ashe::makeCube(scene, 1.f);
        mesh.scale(minScaling + (maxScaling - minScaling) * ashe::random01());
        mesh.translate({ashe::random01() - 1.f, 2.f * ashe::random01() - 1.f, ashe::random01()});
        mesh.rotate({ashe::random01(), ashe::random01(), ashe::random01()}, 3.14f / 4.f * ashe::random01());
    }

    // @fixme DOING THAT BEFORE ADDING MESHES DOES NOT CURRENTLY WORK
    auto& camera = scene.make<magma::Camera>(window.extent());
    magma::OrbitCameraController cameraController(camera);
    cameraController.origin({-4.f, -2.f, 1.f});
    cameraController.target({0, 0, 0});
    engine.addView(camera, renderTarget, {0, 0, 1, 1});

    // @fixme STOLEN FROM ASHE
    static auto buttonPressed = MouseButton::Unknown;
    static glm::vec2 lastDragPosition;

    // Main loop
    while (window.opened()) {
        std::optional<WsEvent> event;
        while ((event = window.pollEvent())) {
            switch (event->type) {
            case WsEventType::WindowClosed: {
                window.close();
                break;
            }
            case WsEventType::KeyPressed: {
                if (event->key.which == Key::Escape) {
                    window.close();
                }
                break;
            }

            // @fixme STOLEN FROM ASHE, COULD BE CALLED
            case WsEventType::MouseButtonPressed: {
                buttonPressed = event->mouseButton.which;
                lastDragPosition.x = event->mouseButton.x;
                lastDragPosition.y = event->mouseButton.y;
                break;
            }

            case WsEventType::MouseButtonReleased: {
                buttonPressed = MouseButton::Unknown;
                break;
            }

            case WsEventType::MouseWheelScrolled: {
                if (event->mouseWheel.which != MouseWheel::Vertical) break;
                cameraController.radiusAdd(-event->mouseWheel.delta * cameraController.radius() / 10.f);
                break;
            }

            case WsEventType::MouseMoved: {
                if (buttonPressed == MouseButton::Unknown) break;

                glm::vec2 translation(event->mouseMove.x, event->mouseMove.y);
                auto delta = (translation - lastDragPosition) / 100.f;
                lastDragPosition = translation;

                // Orbit with left button
                if (buttonPressed == MouseButton::Left) {
                    cameraController.orbitAdd(-delta.x, delta.y);
                }
                // Strafe with right button
                else if (buttonPressed == MouseButton::Right) {
                    cameraController.strafe(delta.x / 10.f, delta.y / 10.f);
                }
                break;
            }
            default: break;
            }
        }

        engine.update();
        engine.draw();
    }

    return EXIT_SUCCESS;
}
