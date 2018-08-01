/**
 * Shows how to set-up scenes and windows.
 */

#include <lava/crater.hpp>
#include <lava/magma.hpp>

using namespace lava;

int main(void)
{
    magma::RenderEngine engine;

    crater::Window smallWindow(crater::VideoMode{400, 400}, "ashe - magma | Scenes and windows I");
    auto& smallWindowRenderTarget = engine.make<magma::WindowRenderTarget>(smallWindow.handle(), smallWindow.extent());

    crater::Window bigWindow(crater::VideoMode{800, 800}, "ashe - magma | Scenes and windows II");
    auto& bigWindowRenderTarget = engine.make<magma::WindowRenderTarget>(bigWindow.handle(), bigWindow.extent());

    // Left scene setup
    {
        auto& scene = engine.make<magma::RenderScene>();

        auto& frontCamera = scene.make<magma::OrbitCamera>(Extent2d{400, 800});
        frontCamera.translation({0, 3.f, 0.5f});
        frontCamera.target({0, 0, 0.5f});

        auto& leftCamera = scene.make<magma::OrbitCamera>(Extent2d{200, 200});
        leftCamera.translation({-5.f, 0, 0});
        leftCamera.target({0, 0, 0});

        auto& upCamera = scene.make<magma::OrbitCamera>(Extent2d{200, 200});
        upCamera.translation({0, 0, 5.f});
        upCamera.target({0, 0, 0});

        // @todo camera.addView(target, viewport) ?
        engine.addView(frontCamera, smallWindowRenderTarget, {0.5, 0, 0.5, 1});
        engine.addView(leftCamera, smallWindowRenderTarget, {0, 0, 0.5, 0.5});
        engine.addView(upCamera, smallWindowRenderTarget, {0, 0.5, 0.5, 0.5});

        engine.addView(frontCamera, bigWindowRenderTarget, {0, 0, 0.5, 1});
    }

    // Right scene setup
    {
        auto& scene = engine.make<magma::RenderScene>();

        auto& frontCamera = scene.make<magma::OrbitCamera>(Extent2d{400, 800});
        frontCamera.translation({0, 5.f, 0});
        frontCamera.target({0, 0, 0});

        engine.addView(frontCamera, bigWindowRenderTarget, {0.5, 0, 0.5, 1});
    }

    // Main loop
    while (smallWindow.opened() && bigWindow.opened()) {
        std::optional<WsEvent> event;
        while ((event = smallWindow.pollEvent()) || (event = bigWindow.pollEvent())) {
            switch (event->type) {
            case WsEventType::WindowClosed: {
                smallWindow.close();
                bigWindow.close();
                break;
            }
            case WsEventType::KeyPressed: {
                if (event->key.which == Key::Escape) {
                    smallWindow.close();
                    bigWindow.close();
                }
                break;
            }
            default: break;
            }
        }

        engine.draw();
    }

    return EXIT_SUCCESS;
}
