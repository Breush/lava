#pragma once

#include "./capsule.hpp"

#include <vulkan/vulkan.hpp>

#include "./proxy.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a VkInstance.
     */
    class Instance {
    public:
        void init(bool debugEnabled);

        // ----- Getters

        Capsule<VkInstance>& capsule() { return m_instance; }

        operator VkInstance() const { return m_instance; }

    protected:
        void createInstance();
        void setupDebug();

        void initApplication(VkInstanceCreateInfo& instanceCreateInfo);
        void initValidationLayers(VkInstanceCreateInfo& instanceCreateInfo);
        void initRequiredExtensions(VkInstanceCreateInfo& instanceCreateInfo);

    private:
        Capsule<VkInstance> m_instance{vkDestroyInstance};
        VkApplicationInfo m_applicationInfo;
        std::vector<const char*> m_extensions;

        // Validation layers
        bool m_debugEnabled = true;
        const std::vector<const char*> m_validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
        Capsule<VkDebugReportCallbackEXT> m_debugReportCallback{m_instance, DestroyDebugReportCallbackEXT};
    };
}
