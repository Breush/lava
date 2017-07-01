/**
 * Shows how to load meshes using magma rendering-engine.
 */

#include <iostream>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(crater::Event& event, magma::RenderWindow& window, magma::OrbitCamera& camera, magma::PointLight& light);

int main(void)
{
    // A render engine is the global manager.
    magma::RenderEngine engine;

    // Create a window we can draw to
    magma::RenderWindow window({800, 600}, "ashe - magma | load meshes");
    engine.add(window);

    // A camera
    auto& camera = engine.make<magma::OrbitCamera>();
    camera.position({0.f, 2.f, 0.75f});
    camera.target({0.f, 0.f, 0.5f});
    camera.viewportRatio(800.f / 600.f);

    // A light
    auto& light = engine.make<magma::PointLight>();
    light.position({5.f, 5.f, 0.f});

    // Create a mesh
    // engine.make<magma::Mesh>("./assets/models/duck.glb");
    engine.make<magma::Mesh>("./assets/models/corset.glb");

    // auto& sphereMesh = engine.make(magma::makers::sphereMeshMaker(32, 0.5));
    // @fixme Allow it to have no material
    // sphereMesh.material(engine.make<magma::MrrMaterial>());

    // Keep running while the window is open
    while (window.opened()) {
        // Treat all events since last frame
        crater::Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window, camera, light);
        }

        engine.update(); // Update the logic
        engine.draw();   // Render the scene
    }

    return EXIT_SUCCESS;
}

void handleEvent(crater::Event& event, magma::RenderWindow& window, magma::OrbitCamera& camera, magma::PointLight& light)
{
    static auto buttonPressed = crater::input::Button::Unknown;
    static glm::vec2 lastDragPosition;

    switch (event.type) {
    case crater::Event::WindowClosed: {
        window.close();
        break;
    }

    case crater::Event::KeyPressed: {
        if (event.key.which == crater::input::Key::Escape) {
            window.close();
        }
        // @todo Write better controls for the light
        else if (event.key.which == crater::input::Key::Right) {
            light.position(light.position() - glm::vec3{0.1f, 0.f, 0.f});
        }
        else if (event.key.which == crater::input::Key::Left) {
            light.position(light.position() + glm::vec3{0.1f, 0.f, 0.f});
        }
        else if (event.key.which == crater::input::Key::Up) {
            light.position(light.position() - glm::vec3{0.f, 0.1f, 0.f});
        }
        else if (event.key.which == crater::input::Key::Down) {
            light.position(light.position() + glm::vec3{0.f, 0.1f, 0.f});
        }
        break;
    }

    case crater::Event::WindowResized: {
        window.refresh();
        camera.viewportRatio(static_cast<float>(event.size.width) / static_cast<float>(event.size.height));
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
        camera.radiusAdd(-event.mouseScroll.delta / 10.f);
        break;
    }

    case crater::Event::MouseMoved: {
        if (buttonPressed == crater::input::Button::Unknown) return;

        glm::vec2 position(event.mouseMove.x, event.mouseMove.y);
        auto delta = (position - lastDragPosition) / 100.f;
        lastDragPosition = position;

        // Orbit with left button
        if (buttonPressed == crater::input::Button::Left) {
            camera.orbitAdd(-delta.x, -delta.y);
        }
        // Strafe with right button
        else if (buttonPressed == crater::input::Button::Right) {
            camera.strafe(delta.x / 10.f, delta.y / 10.f);
        }
        break;
    }

    default: break;
    }
}
