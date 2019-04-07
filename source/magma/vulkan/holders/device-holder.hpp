#pragma once

#include "../helpers/queue.hpp"
#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::Device.
     */
    class DeviceHolder {
    public:
        /// pSurface can be set to nullptr if the application does not draw to a window surface.
        void init(vk::Instance instance, vk::SurfaceKHR* pSurface, bool debugEnabled, bool vrEnabled);

        void debugObjectName(vk::DescriptorSet object, const std::string& name) const;
        void debugObjectName(vk::ImageView object, const std::string& name) const;
        void debugObjectName(vk::Image object, const std::string& name) const;
        void debugObjectName(vk::Semaphore object, const std::string& name) const;

        void debugBeginRegion(vk::CommandBuffer commandBuffer, const std::string& name) const;
        void debugEndRegion(vk::CommandBuffer commandBuffer) const;

        // ----- Getters

        const vk::Device& device() const { return m_device; }
        const vk::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }
        const vk::Queue& graphicsQueue() const { return m_graphicsQueue; }
        const vk::Queue& presentQueue() const { return m_presentQueue; }
        uint32_t graphicsQueueFamilyIndex() const { return m_queueFamilyIndices.graphics; }
        uint32_t presentQueueFamilyIndex() const { return m_queueFamilyIndices.present; }

        const std::vector<const char*>& extensions() const { return m_extensions; }

    protected:
        void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR* pSurface);
        void createLogicalDevice(vk::SurfaceKHR* pSurface);

        /// Generic function, make a public override if needed.
        void debugObjectName(uint64_t object, vk::ObjectType objectType, const std::string& name) const;

    private:
        vulkan::Device m_device;

        vk::PhysicalDevice m_physicalDevice = nullptr;
        vk::Queue m_graphicsQueue = nullptr;
        vk::Queue m_presentQueue = nullptr;
        QueueFamilyIndices m_queueFamilyIndices;

        const std::vector<const char*> m_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        bool m_debugEnabled = false; // Should be in sync with InstanceHolder.
        bool m_vrEnabled = false;    // Should be in sync with InstanceHolder.
    };
}
