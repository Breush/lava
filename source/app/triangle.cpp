
#include <iostream>
#include <lava/crater/Event.hpp>
#include <lava/magma.hpp>

using namespace lava;

void handleEvent(lava::Event& event);
void update();

int main(void)
{
    // Create a window, holding our scene
    lava::Window window({800, 600}, "The best example");

    // An engine is the global manager.
    // A scene is our 3D environment, and set it to be shown in the window
    lava::Engine engine;
    lava::Scene scene(engine, window);

    // Keep running while the window is open
    while (window.isOpen()) {
        // Treat all events since last frame
        lava::Event event;
        while (window.pollEvent(event)) {
            handleEvent(event);
        }

        // Update the logic
        update();

        // Render the scene
        scene.render();
    }

    return EXIT_SUCCESS;
}

void handleEvent(lava::Event& event)
{
}

void update()
{
}
