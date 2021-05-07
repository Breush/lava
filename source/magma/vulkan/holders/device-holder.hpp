#pragma once

#include "../helpers/queue.hpp"
#include "../wrappers.hpp"

namespace lava::magma {
    class VrEngine;
}

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::Device.
     */
    class DeviceHolder {
    public:
        /// pSurface can be set to nullptr if the application does not draw to a window surface.
        void init(vk::Instance instance, vk::SurfaceKHR* pSurface, bool debugEnabled, VrEngine& vr);

        void debugObjectName(vk::DescriptorSet object, const std::string& name) const;
        void debugObjectName(vk::ImageView object, const std::string& name) const;
        void debugObjectName(vk::Image object, const std::string& name) const;
        void debugObjectName(vk::Semaphore object, const std::string& name) const;
        void debugObjectName(vk::DeviceMemory object, const std::string& name) const;

        void debugBeginRegion(vk::CommandBuffer commandBuffer, const std::string& name) const;
        void debugEndRegion(vk::CommandBuffer commandBuffer) const;

        // ----- Getters

        const vk::Device& device() const { return m_device.get(); }
        const vk::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }
        const vk::Queue& graphicsQueue() const { return m_graphicsQueue; }
        const vk::Queue& transferQueue() const { return m_transferQueue; }
        const vk::Queue& presentQueue() const { return m_presentQueue; }
        uint32_t graphicsQueueFamilyIndex() const { return m_queueFamilyIndices.graphics; }
        uint32_t presentQueueFamilyIndex() const { return m_queueFamilyIndices.present; }
        vk::SampleCountFlagBits maxSampleCount() const { return m_maxSampleCount; }
        const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& rtPipelineProperties() const { return m_rtPipelineProperties; }

        const std::vector<const char*>& extensions() const { return m_extensions; }

    protected:
        void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR* pSurface);
        void createLogicalDevice(vk::SurfaceKHR* pSurface, VrEngine& vr);

        /// Generic function, make a public override if needed.
        void debugObjectName(uint64_t object, vk::ObjectType objectType, const std::string& name) const;

    private:
        vk::UniqueDevice m_device;

        vk::PhysicalDevice m_physicalDevice = nullptr;
        vk::Queue m_graphicsQueue = nullptr;
        vk::Queue m_transferQueue = nullptr;
        vk::Queue m_presentQueue = nullptr;
        QueueFamilyIndices m_queueFamilyIndices;
        vk::SampleCountFlagBits m_maxSampleCount = vk::SampleCountFlagBits::e1;
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtPipelineProperties;

        const std::vector<const char*> m_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        bool m_debugEnabled = false; // Should be in sync with InstanceHolder.
    };
}
