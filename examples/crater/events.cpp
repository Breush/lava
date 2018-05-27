/**
 * Shows how to create a window and handle events.
 */

#include <iostream>
#include <lava/crater.hpp>

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
                std::cout << "    x: " << event->mouseButton.x << std::endl;
                std::cout << "    y: " << event->mouseButton.y << std::endl;
                std::cout << "    which: " << event->mouseButton.which << std::endl;
                break;
            }
            case WsEventType::MouseButtonReleased: {
                std::cout << "    x: " << event->mouseButton.x << std::endl;
                std::cout << "    y: " << event->mouseButton.y << std::endl;
                std::cout << "    which: " << event->mouseButton.which << std::endl;
                break;
            }
            case WsEventType::MouseScroll: {
                std::cout << "    x: " << event->mouseScroll.x << std::endl;
                std::cout << "    y: " << event->mouseScroll.y << std::endl;
                std::cout << "    delta: " << event->mouseScroll.delta << std::endl;
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
