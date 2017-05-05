
#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(lava::Event& event, bool& quit);
void update();

int main(void)
{
    // Create a window, holding our scene
    lava::Window window({800, 600}, "The best example");

    // An engine is the global manager.
    // A scene is our 3D environment, and set it to be shown in the window
    lava::Engine engine(window);
    lava::Scene scene(engine);

    // Keep running while the window is open
    bool quit = false;
    while (window.isOpen()) {
        // Treat all events since last frame
        lava::Event event;
        while (window.pollEvent(event)) {
            handleEvent(event, quit);
            if (quit) {
                window.close();
                continue;
            }
        }

        // Update the logic
        update();

        // Render the scene
        // scene.Render();
        engine.draw();
    }

    return EXIT_SUCCESS;
}

void handleEvent(lava::Event& event, bool& quit)
{
    switch (event.type) {
    case lava::Event::WindowClosed: {
        quit = true;
        break;
    }

    case lava::Event::KeyPressed: {
        if (event.key.which == lava::Keyboard::Escape) quit = true;
    }

    default: break;
    }
}

void update()
{
}
