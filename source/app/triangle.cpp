#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(Event& event, RenderWindow& window, OrbitCamera& camera);

int main(void)
{
    // A render engine is the global manager.
    RenderEngine engine;

    // Create a window we can draw to
    RenderWindow window({800, 600}, "The best example");
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
        Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window, camera);
        }

        engine.update(); // Update the logic
        engine.draw();   // Render the scene
    }

    return EXIT_SUCCESS;
}

void handleEvent(Event& event, RenderWindow& window, OrbitCamera& camera)
{
    static auto buttonPressed = lava::Mouse::Unknown;
    static glm::vec2 lastDragPosition;

    switch (event.type) {
    case Event::WindowClosed: {
        window.close();
        break;
    }

    case Event::KeyPressed: {
        if (event.key.which == Keyboard::Escape) {
            window.close();
            break;
        }
    }

    case Event::WindowResized: {
        window.refresh();
        camera.viewportRatio(static_cast<float>(event.size.width) / static_cast<float>(event.size.height));
        break;
    }

    case Event::MouseButtonPressed: {
        buttonPressed = event.mouseButton.which;
        lastDragPosition.x = event.mouseButton.x;
        lastDragPosition.y = event.mouseButton.y;
        break;
    }

    case Event::MouseButtonReleased: {
        buttonPressed = lava::Mouse::Unknown;
        break;
    }

    case Event::MouseScrolled: {
        camera.radiusAdd(-event.mouseScroll.delta / 10.f);
        break;
    }

    case Event::MouseMoved: {
        if (buttonPressed == lava::Mouse::Unknown) return;

        glm::vec2 position(event.mouseMove.x, event.mouseMove.y);
        auto delta = (position - lastDragPosition) / 100.f;
        lastDragPosition = position;

        // Orbit with left button
        if (buttonPressed == lava::Mouse::Left) {
            camera.orbitAdd(-delta.x, -delta.y);
        }
        // Strafe with right button
        else if (buttonPressed == lava::Mouse::Right) {
            camera.strafe(delta.x / 10.f, delta.y / 10.f);
        }
        break;
    }

    default: break;
    }
}
