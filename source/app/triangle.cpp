#include "./vulkan-example.hpp"

int main(void)
{
    lava::Window window(lava::VideoMode(800, 600), "My window");

    VulkanExample vulkanExample(window);
    vulkanExample.initVulkan();
    vulkanExample.setupWindow();
    vulkanExample.initSwapchain();
    vulkanExample.prepare();

    while (window.isOpen()) {
        lava::Event event;
        while (window.pollEvent(event)) {
            vulkanExample.handleLavaEvent(event);
        }

        vulkanExample.update();
        vulkanExample.render();
    }

    return 0;
}
