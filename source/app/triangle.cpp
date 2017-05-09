#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(Event& event, Window& window, Engine& engine);

int main(void)
{
    // Create a window, holding our scene
    Window window({800, 600}, "The best example");

    // An engine is the global manager.
    // A scene is our 3D environment, and set it to be shown in the window
    Engine engine(window);

    // Create a mesh
    Mesh mesh(engine);
    mesh.verticesCount(4);
    mesh.verticesPositions({{-1.f, -1.f, 0.25f}, {1.f, -1.f, 0.25f}, {1.f, 1.f, 0.25f}, {-1.f, 1.f, 0.25f}});
    mesh.verticesColors({{1.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 1.f}, {1.f, 1.f, 1.f}});
    mesh.indices({0, 1, 2, 2, 3, 0});

    Mesh mesh2(engine);
    mesh2.verticesCount(4);
    mesh2.verticesPositions({{-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f}, {1.f, 1.f, 0.f}, {-1.f, 1.f, 0.f}});
    mesh2.verticesColors({{1.f, 0.f, 0.f}, {1.f, 0.f, 1.f}, {1.f, 1.f, 1.f}, {1.f, 1.f, 0.f}});
    mesh2.indices({0, 1, 2, 2, 3, 0});

    // Keep running while the window is open
    while (window.isOpen()) {
        // Treat all events since last frame
        Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window, engine);
        }

        engine.update(); // Update the logic
        engine.draw();   // Render the scene
    }

    return EXIT_SUCCESS;
}

void handleEvent(Event& event, Window& window, Engine& engine)
{
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
        engine.mode(window.videoMode());
        break;
    }

    default: break;
    }
}
