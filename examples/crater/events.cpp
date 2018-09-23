/**
 * Shows how to create a window and handle events.
 */

#include <lava/crater.hpp>

#include <iostream>

using namespace lava;

int main(void)
{
    crater::Window window(Extent2d{400, 300}, "Events");

    while (window.opened()) {
        while (auto event = window.pollEvent()) {
            std::cout << event->type << std::endl;

            switch (event->type) {
            case WsEventType::WindowClosed: {
                window.close();
                break;
            }
            case WsEventType::WindowResized: {
                std::cout << "    width: " << event->windowSize.width << std::endl;
                std::cout << "    height: " << event->windowSize.height << std::endl;
                break;
            }
            case WsEventType::MouseButtonPressed: {
                std::cout << "    which: " << event->mouseButton.which << std::endl;
                std::cout << "    x: " << event->mouseButton.x << std::endl;
                std::cout << "    y: " << event->mouseButton.y << std::endl;
                break;
            }
            case WsEventType::MouseButtonReleased: {
                std::cout << "    which: " << event->mouseButton.which << std::endl;
                std::cout << "    x: " << event->mouseButton.x << std::endl;
                std::cout << "    y: " << event->mouseButton.y << std::endl;
                break;
            }
            case WsEventType::MouseWheelScrolled: {
                std::cout << "    which: " << event->mouseWheel.which << std::endl;
                std::cout << "    delta: " << event->mouseWheel.delta << std::endl;
                std::cout << "    x: " << event->mouseWheel.x << std::endl;
                std::cout << "    y: " << event->mouseWheel.y << std::endl;
                break;
            }
            case WsEventType::MouseMoved: {
                std::cout << "    x: " << event->mouseMove.x << std::endl;
                std::cout << "    y: " << event->mouseMove.y << std::endl;
                break;
            }
            case WsEventType::KeyPressed: {
                std::cout << "    which: " << event->key.which << std::endl;
                break;
            }
            case WsEventType::KeyReleased: {
                std::cout << "    which: " << event->key.which << std::endl;
                break;
            }
            }
        }
    }

    return EXIT_SUCCESS;
}
