#pragma once

#include "../wrappers.hpp"

namespace lava::magma::vulkan {
    /**
     * An abstraction over a vk::Instance.
     */
    class InstanceHolder final {
    public:
        void init(bool debugEnabled);

        // ----- Getters

        const vk::Instance& instance() const { return m_instance; }
        bool debugEnabled() const { return m_debugEnabled; }

    protected:
        void createInstance();
        void setupDebug();

        void initApplication(vk::InstanceCreateInfo& instanceCreateInfo);
        void initValidationLayers(vk::InstanceCreateInfo& instanceCreateInfo);
        void initRequiredExtensions(vk::InstanceCreateInfo& instanceCreateInfo);

    private:
        // Resources
        vulkan::Instance m_instance;
        vulkan::DebugUtilsMessengerEXT m_debugUtilsMessenger{m_instance};

        // Application
        vk::ApplicationInfo m_applicationInfo;
        std::vector<const char*> m_extensions;

        // Validation layers
        bool m_debugEnabled = true;
        const std::vector<const char*> m_validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    };
}
