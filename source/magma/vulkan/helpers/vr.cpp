#include "./vr.hpp"

#include <lava/chamber/string-tools.hpp>

using namespace lava::magma;

const std::vector<std::string>& vulkan::vrRequiredInstanceExtensions()
{
    static std::vector<std::string> extensions;
    if (!extensions.empty()) {
        return extensions;
    }

    uint32_t bufferSize = vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(nullptr, 0);

    std::string extensionsString;
    extensionsString.resize(bufferSize);

    vr::VRCompositor()->GetVulkanInstanceExtensionsRequired(extensionsString.data(), bufferSize);

    return (extensions = chamber::split(extensionsString, ' '));
}

const std::vector<std::string>& vulkan::vrRequiredDeviceExtensions(vk::PhysicalDevice physicalDevice)
{
    static std::vector<std::string> extensions;
    if (!extensions.empty()) {
        return extensions;
    }

    uint32_t bufferSize = vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalDevice, nullptr, 0);

    std::string extensionsString;
    extensionsString.resize(bufferSize);

    vr::VRCompositor()->GetVulkanDeviceExtensionsRequired(physicalDevice, extensionsString.data(), bufferSize);

    return (extensions = chamber::split(extensionsString, ' '));
}
