#pragma once

#include "./capsule.hpp"

#include <lava/crater/WindowHandle.hpp>
#include <vulkan/vulkan.hpp>

namespace lava::vulkan {
    class Instance;
}

namespace lava::vulkan {
    /**
     * An abstraction over a VkSurfaceKHR.
     */
    class Surface {
    public:
        Surface(Instance& instance);

        void init(WindowHandle& windowHandle);

        // ----- Getters

        Capsule<VkSurfaceKHR>& capsule() { return m_surface; }

        operator VkSurfaceKHR() const { return m_surface; }

    protected:
        void createSurface(WindowHandle& windowHandle);

    private:
        Capsule<VkSurfaceKHR> m_surface;

        Instance& m_instance;
    };
}
