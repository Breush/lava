#include "./surface.hpp"

#include <lava/chamber/logger.hpp>

#include "./instance.hpp"

using namespace lava::magma::vulkan;
using namespace lava::crater;
using namespace lava::chamber;

Surface::Surface(Instance& instance)
    : m_surface(instance.capsule(), vkDestroySurfaceKHR)
    , m_instance(instance)
{
}

void Surface::init(WindowHandle& windowHandle)
{
    createSurface(windowHandle);
}

void Surface::createSurface(WindowHandle& windowHandle)
{
    // @todo This is platform-specific!
    VkXcbSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.connection = windowHandle.connection;
    createInfo.window = windowHandle.window;

    auto CreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(m_instance.capsule(), "vkCreateXcbSurfaceKHR");
    auto err = CreateXcbSurfaceKHR(m_instance, &createInfo, nullptr, m_surface.replace());
    if (err) {
        logger.error("magma.vulkan.surface") << "Unable to create surface for platform." << std::endl;
    }
}
