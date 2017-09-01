/**
 * Shows how to set-up scenes and windows.
 */

#include <lava/magma.hpp>

using namespace lava;

int main(void)
{
    magma::RenderEngine engine;

    auto& smallWindow = engine.make<magma::RenderWindow>(crater::VideoMode{400, 400}, "ashe - magma | Scenes and windows I");
    auto& bigWindow = engine.make<magma::RenderWindow>(crater::VideoMode{800, 800}, "ashe - magma | Scenes and windows II");

    // Corset scene setup
    {
        auto& corsetScene = engine.make<magma::RenderScene>();
        corsetScene.make<magma::Mesh>("./assets/models/corset.glb");

        auto& light = corsetScene.make<magma::PointLight>();
        light.position({2.f, 2.f, 2.f});

        auto& frontCamera = corsetScene.make<magma::OrbitCamera>(magma::Extent2d{400, 800});
        frontCamera.position({0, 3.f, 0.5f});
        frontCamera.target({0, 0, 0.5f});

        auto& leftCamera = corsetScene.make<magma::OrbitCamera>(magma::Extent2d{200, 200});
        leftCamera.position({-5.f, 0, 0});
        leftCamera.target({0, 0, 0});

        auto& upCamera = corsetScene.make<magma::OrbitCamera>(magma::Extent2d{200, 200});
        upCamera.position({0, 0, 5.f});
        upCamera.target({0, 0, 0});

        // @fixme camera.addView(target, viewport) ?
        engine.addView(frontCamera, smallWindow, {0.5, 0, 0.5, 1});
        engine.addView(leftCamera, smallWindow, {0, 0, 0.5, 0.5});
        engine.addView(upCamera, smallWindow, {0, 0.5, 0.5, 0.5});

        engine.addView(frontCamera, bigWindow, {0, 0, 0.5, 1});
    }

    // Sphere scene setup
    {
        auto& sphereScene = engine.make<magma::RenderScene>();

        auto& mesh = sphereScene.make(magma::makers::sphereMeshMaker(32, 0.5));
        // @todo Currently can't have a mesh with no material
        mesh.material(sphereScene.make<magma::RmMaterial>());

        auto& light = sphereScene.make<magma::PointLight>();
        light.position({-2.f, -2.f, 5.f});

        auto& frontCamera = sphereScene.make<magma::OrbitCamera>(magma::Extent2d{400, 800});
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

        engine.update();
        engine.draw();
    }

    return EXIT_SUCCESS;
}
