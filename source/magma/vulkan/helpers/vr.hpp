#pragma once

#include <lava/magma/vr-tools.hpp>

#include <string>
#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /// Get the list of all required instance extensions so that VR can work.
    const std::vector<std::string>& vrRequiredInstanceExtensions();

    /// Get the list of all required device extensions so that VR can work.
    const std::vector<std::string>& vrRequiredDeviceExtensions(vk::PhysicalDevice physicalDevice);
}
