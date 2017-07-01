/**
 * Shows how to load meshes using magma rendering-engine.
 */

#include <iostream>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(crater::Event& event, RenderWindow& window, OrbitCamera& camera);

int main(void)
{
    // A render engine is the global manager.
    RenderEngine engine;

    // Create a window we can draw to
    RenderWindow window({800, 600}, "ashe - magma | load meshes");
    engine.add(window);

    auto& camera = engine.make<OrbitCamera>();
    camera.position({0.f, 2.f, 0.75f});
    camera.target({0.f, 0.f, 0.5f});
    camera.viewportRatio(800.f / 600.f);

    // Create a mesh
    // engine.make<Mesh>("./assets/models/duck.glb");
    engine.make<Mesh>("./assets/models/corset.glb");

    // auto& sphereMesh = engine.make(lava::makers::sphereMeshMaker(32, 0.5));
    // @fixme Allow it to have no material
    // sphereMesh.material(engine.make<MrrMaterial>());

    // Keep running while the window is open
    while (window.opened()) {
        // Treat all events since last frame
        crater::Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window, camera);
        }

        engine.update(); // Update the logic
        engine.draw();   // Render the scene
    }

    return EXIT_SUCCESS;
}

void handleEvent(crater::Event& event, RenderWindow& window, OrbitCamera& camera)
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
            break;
        }
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
