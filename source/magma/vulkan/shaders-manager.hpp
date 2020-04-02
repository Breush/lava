#pragma once

#include "./wrappers.hpp"

namespace lava::magma {

    class ShadersManager final {
    public:
        struct ModuleOptions {
            std::unordered_map<std::string, std::string> defines;
            std::function<void(void)> updateCallback = nullptr;
        };

    public:
        ShadersManager(const vk::Device& device);

        /// Register multiple impls thanks to files annotations.
        void registerImplGroup(const std::string& hrid, const std::string& rawCode, uint32_t implsId = -1u);

        /// Update previously registered impl group.
        void updateImplGroup(const std::string& hrid, const std::string& rawCode);

        /// Get a module from an id.
        vk::ShaderModule module(const std::string& shaderId);

        /**
         * Get a module from an id and set values of define.
         *
         * Please note that a same shader can only have its
         * values defined once. Getting a previously defined shaderId
         * will get the previous one, even if defines are different.
         */
        vk::ShaderModule module(const std::string& shaderId, const ModuleOptions& options);

    protected:
        struct Impl {
            std::vector<std::string> textCodes;
            std::vector<uint32_t> ids;
            std::set<std::string> dirtyShaderIds;
            std::vector<std::function<void(void)>> updateCallbacks;
        };

        struct ImplGroup {
            std::unordered_map<std::string, uint32_t> implIds;
        };

        struct ResolvedShader {
            std::string textCode;
            std::set<std::string> implsDependencies;
        };

        struct ModuleInfo {
            std::unique_ptr<vulkan::ShaderModule> module;
            std::set<std::string> implsDependencies;
        };

    protected:
        /// All the concerned modules will be warned.
        void dirtifyImpl(const std::string& category);

        /// Register an impl, its id is returned. Expects the implCode to have no annotations.
        uint32_t registerImpl(const std::string& category, const std::string& implCode, uint32_t implId);

        /// Update a previously registered impl.
        void updateImpl(const std::string& category, const uint32_t implId, const std::string& implCode);

        /// Transform the provided impl code.
        ResolvedShader resolveImpl(const std::string& category, const uint32_t implId, const std::string& implCode);

        /// Transform magma annotations to glsl code.
        ResolvedShader resolveShader(const std::string& textCode, std::string annotationMain = "");

        /// Find magma annotations for impls and creates an array of it.
        std::vector<std::pair<std::string, std::string>> extractImpls(const std::string& rawCode);

    private:
        // References
        const vk::Device& m_device;

        // Data
        std::set<std::string> m_dirtyModules;
        std::unordered_map<std::string, Impl> m_impls;
        std::unordered_map<std::string, ImplGroup> m_implGroups;
        std::unordered_map<std::string, ModuleInfo> m_modulesInfos;
    };
}
