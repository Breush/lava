#pragma once

#include <lava/chamber/macros.hpp>

#include "./wrappers.hpp"

namespace lava::magma::vulkan {
    class Device;
}

namespace lava::magma::vulkan {
    /**
     * Simple wrapper around a vulkna ImageView,
     * holding its device memory and such.
     */
    class ImageHolder {
    public:
        ImageHolder(Device& device);

        /**
         * Allocate all image memory for the specified format.
         */
        void create(vk::Format format, vk::Extent2D extent, vk::ImageAspectFlagBits imageAspect);

    private:
        // References
        vulkan::Device& m_device;

        // Depth
        $attribute(vulkan::Image, image);
        $attribute(vulkan::DeviceMemory, memory);
        $attribute(vulkan::ImageView, view);
    };
}
