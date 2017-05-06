
#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(Event& event, Window& window, Engine& engine);
void update();

int main(void)
{
    // Create a window, holding our scene
    Window window({800, 600}, "The best example");

    // An engine is the global manager.
    // A scene is our 3D environment, and set it to be shown in the window
    Engine engine(window);
    Scene scene(engine);

    // Keep running while the window is open
    while (window.isOpen()) {
        // Treat all events since last frame
        Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, window, engine);
        }

        // Update the logic
        update();

        // Render the scene
        // scene.Render();
        engine.draw();
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

void update()
{
}
