#pragma once

#include <string>
#include <unordered_map>

#include "./wrappers.hpp"

namespace lava::magma {
    class ShadersManager final {
    public:
        ShadersManager(const vk::Device& device);

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

    private:
        // References
        const vk::Device& m_device;

        // Data
        std::unordered_map<std::string, vulkan::ShaderModule> m_modules;
    };
}
