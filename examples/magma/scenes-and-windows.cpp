/**
 * Shows how to set-up scenes and windows.
 */

#include <lava/magma.hpp>

using namespace lava;

int main(void)
{
    magma::RenderEngine engine;
    engine.registerMaterial<magma::RmMaterial>();

    auto& smallWindow = engine.make<magma::RenderWindow>(crater::VideoMode{400, 400}, "ashe - magma | Scenes and windows I");
    auto& bigWindow = engine.make<magma::RenderWindow>(crater::VideoMode{800, 800}, "ashe - magma | Scenes and windows II");

    // Left scene setup
    {
        auto& scene = engine.make<magma::RenderScene>();

        auto& frontCamera = scene.make<magma::OrbitCamera>(Extent2d{400, 800});
        frontCamera.position({0, 3.f, 0.5f});
        frontCamera.target({0, 0, 0.5f});

        auto& leftCamera = scene.make<magma::OrbitCamera>(Extent2d{200, 200});
        leftCamera.position({-5.f, 0, 0});
        leftCamera.target({0, 0, 0});

        auto& upCamera = scene.make<magma::OrbitCamera>(Extent2d{200, 200});
        upCamera.position({0, 0, 5.f});
        upCamera.target({0, 0, 0});

        // @todo camera.addView(target, viewport) ?
        engine.addView(frontCamera, smallWindow, {0.5, 0, 0.5, 1});
        engine.addView(leftCamera, smallWindow, {0, 0, 0.5, 0.5});
        engine.addView(upCamera, smallWindow, {0, 0.5, 0.5, 0.5});

        engine.addView(frontCamera, bigWindow, {0, 0, 0.5, 1});
    }

    // Right scene setup
    {
        auto& scene = engine.make<magma::RenderScene>();

        auto& frontCamera = scene.make<magma::OrbitCamera>(Extent2d{400, 800});
        frontCamera.position({0, 5.f, 0});
        frontCamera.target({0, 0, 0});

        engine.addView(frontCamera, bigWindow, {0.5, 0, 0.5, 1});
    }

    // Main loop
    while (smallWindow.opened() && bigWindow.opened()) {
        crater::Event event;
        while (smallWindow.pollEvent(event) || bigWindow.pollEvent(event)) {
            switch (event.type) {
            case crater::Event::WindowClosed: {
                smallWindow.close();
                bigWindow.close();
                break;
            }
            case crater::Event::KeyPressed: {
                if (event.key.which == crater::input::Key::Escape) {
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
