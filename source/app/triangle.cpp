#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(Event& event, RenderWindow& window);

int main(void)
{
    // A render engine is the global manager.
    RenderEngine engine;

    // Create a window we can draw to
    RenderWindow window({800, 600}, "The best example");
    engine.add(window);

    // Create a mesh
    // engine.make<Mesh>("./assets/models/duck.glb");
    engine.make<Mesh>("./assets/models/corset.glb");
    // engine.make(lava::makers::sphereMeshMaker(32, 0.5));

    // Keep running while the window is open
    while (window.opened()) {
        // Treat all events since last frame
        Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window);
        }

        engine.update(); // Update the logic
        engine.draw();   // Render the scene
    }

    return EXIT_SUCCESS;
}

void handleEvent(Event& event, RenderWindow& window)
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
        window.refresh();
        break;
    }

    default: break;
    }
}
