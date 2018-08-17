#pragma once

#include <set>
#include <vulkan/vulkan.hpp>

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::Device.
     */
    class DeviceHolder {
    public:
        void init(vk::Instance instance, vk::SurfaceKHR surface, bool debugEnabled);

        void debugObjectName(vk::DescriptorSet object, const std::string& name) const;

        void debugBeginRegion(vk::CommandBuffer commandBuffer, const std::string& name) const;
        void debugEndRegion(vk::CommandBuffer commandBuffer) const;

        // ----- Getters

        const vk::Device& device() const { return m_device; }
        const vk::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }
        const vk::Queue& graphicsQueue() const { return m_graphicsQueue; }
        const vk::Queue& presentQueue() const { return m_presentQueue; }

        const std::vector<const char*>& extensions() const { return m_extensions; }

    protected:
        void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
        void createLogicalDevice(vk::SurfaceKHR surface);

        /// Generic function, make a public override if needed.
        void debugObjectName(uint64_t object, vk::ObjectType objectType, const std::string& name) const;

    private:
        vulkan::Device m_device;

        vk::PhysicalDevice m_physicalDevice = nullptr;
        vk::Queue m_graphicsQueue = nullptr;
        vk::Queue m_presentQueue = nullptr;

        const std::vector<const char*> m_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        bool m_debugEnabled = false; // Should be in sync with InstanceHolder.
    };
}
