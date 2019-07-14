/**
 * Shows how to set-up scenes and windows.
 */

#include <lava/crater.hpp>
#include <lava/magma.hpp>

#include "./ashe.hpp"

using namespace lava;

int main(void)
{
    magma::RenderEngine engine;

    crater::Window smallWindow(crater::VideoMode{400, 400}, "ashe - magma | Scenes and windows I");
    auto& smallWindowRenderTarget = engine.make<magma::WindowRenderTarget>(smallWindow.handle(), smallWindow.extent());

    crater::Window bigWindow(crater::VideoMode{800, 800}, "ashe - magma | Scenes and windows II");
    auto& bigWindowRenderTarget = engine.make<magma::WindowRenderTarget>(bigWindow.handle(), bigWindow.extent());

    // Left scene setup, with a cube
    {
        auto& scene = engine.make<magma::RenderScene>();
        scene.rendererType(magma::RendererType::DeepDeferred);

        auto& light = scene.make<magma::DirectionalLight>();
        light.direction({-0.8f, -0.7f, -0.4f});

        auto& frontCamera = scene.make<magma::Camera>(Extent2d{400, 800});
        magma::OrbitCameraController frontCameraController(frontCamera);
        frontCameraController.origin({5.f, 0, 0});
        frontCameraController.target({0, 0, 0});

        auto& leftCamera = scene.make<magma::Camera>(Extent2d{200, 200});
        magma::OrbitCameraController leftCameraController(leftCamera);
        leftCameraController.origin({0, 5.f, 0});
        leftCameraController.target({0, 0, 0});

        auto& upCamera = scene.make<magma::Camera>(Extent2d{200, 200});
        magma::OrbitCameraController upCameraController(upCamera);
        upCameraController.origin({0.01f, 0.01f, 5.f});
        upCameraController.target({0, 0, 0});

        auto& cube = ashe::makeCube(scene, 0.7f);
        cube.rotate({1.f, 0.f, 1.f}, 3.14f / 3.f);

        // @todo camera.addView(target, viewport) ?
        engine.addView(frontCamera, smallWindowRenderTarget, {0.5, 0, 0.5, 1});
        engine.addView(leftCamera, smallWindowRenderTarget, {0, 0, 0.5, 0.5});
        engine.addView(upCamera, smallWindowRenderTarget, {0, 0.5, 0.5, 0.5});

        engine.addView(frontCamera, bigWindowRenderTarget, {0, 0, 0.5, 1});
    }

    // Right scene setup
    {
        auto& scene = engine.make<magma::RenderScene>();
        scene.rendererType(magma::RendererType::DeepDeferred);

        auto& light = scene.make<magma::DirectionalLight>();
        light.direction({-0.8f, -0.7f, -0.4f});

        auto& frontCamera = scene.make<magma::Camera>(Extent2d{400, 800});
        magma::OrbitCameraController frontCameraController(frontCamera);
        frontCameraController.origin({5.f, 0, 0});
        frontCameraController.target({0, 0, 0});

        auto& tetrahedron = ashe::makeTetrahedron(scene, 0.7f);
        tetrahedron.rotate({1.f, 0.f, 1.f}, 3.14f / 3.f);

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

        engine.update();
        engine.draw();
    }

    return EXIT_SUCCESS;
}
