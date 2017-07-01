#pragma once

#include <lava/crater/window-handle.hpp>
#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

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

        void init(crater::WindowHandle& windowHandle);

        // ----- Getters

        Capsule<VkSurfaceKHR>& capsule() { return m_surface; }

        operator VkSurfaceKHR() const { return m_surface; }

    protected:
        void createSurface(crater::WindowHandle& windowHandle);

    private:
        Capsule<VkSurfaceKHR> m_surface;

        Instance& m_instance;
    };
}
