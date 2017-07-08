#pragma once

#include <set>
#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a VkDevice.
     */
    class Device {
    public:
        void init(VkInstance instance, VkSurfaceKHR surface);

        // ----- Getters

        Capsule<VkDevice>& capsule() { return m_device; }
        const VkPhysicalDevice& physicalDevice() const { return m_physicalDevice; }
        const std::vector<const char*>& extensions() const { return m_extensions; }
        const VkQueue& graphicsQueue() const { return m_graphicsQueue; }
        const VkQueue& presentQueue() const { return m_presentQueue; }

        operator VkDevice() const { return m_device; }

        // @todo Should make a cast operator
        const vk::Device& vk() const { return reinterpret_cast<const vk::Device&>(m_device); }

    protected:
        void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        void createLogicalDevice(VkSurfaceKHR surface);

    private:
        Capsule<VkDevice> m_device{vkDestroyDevice};
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        const std::vector<const char*> m_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
    };
}
