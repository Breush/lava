#pragma once

#include <string>
#include <unordered_map>

#include "./wrappers.hpp"

namespace lava::magma {
    class ShadersManager final {
    public:
        ShadersManager(const vk::Device& device);

        /// Register an impl, its id is returned.
        uint32_t registerImpl(const std::string& category, const std::string& implCode);

        /// Register multiple impls thanks to files annotations.
        void registerImpls(const std::string& rawCode);

        /// Get a module from an id.
        vk::ShaderModule module(const std::string& shaderId);

        /**
         * Get a module from an id and set values of define.
         *
         * Please note that a same shader can only have its
         * values defined once. Getting a previously defined shaderId
         * will get the previous one, even if defines are different.
         */
        vk::ShaderModule module(const std::string& shaderId, const std::unordered_map<std::string, std::string>& defines);

    protected:
        /// Transform magma annotations to glsl code.
        std::string resolve(const std::string& textCode, std::string annotationMain = "");

        /// Find magma annotations for impls and creates an array of it.
        std::vector<std::pair<std::string, std::string>> extractImpls(const std::string& rawCode);

    private:
        // References
        const vk::Device& m_device;

        // Data
        std::unordered_map<std::string, std::vector<std::string>> m_impls;
        std::unordered_map<std::string, vulkan::ShaderModule> m_modules;
    };
}
