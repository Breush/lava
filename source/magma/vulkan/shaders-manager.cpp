#include "./shaders-manager.hpp"

#include "../helpers/shader.hpp"
#include "./helpers/shader.hpp"

#include <iostream>

using namespace lava::magma;

ShadersManager::ShadersManager(const vk::Device& device)
    : m_device(device)
{
}

vk::ShaderModule ShadersManager::module(const std::string& shaderId)
{
    return module(shaderId, {});
}

vk::ShaderModule ShadersManager::module(const std::string& shaderId, const std::unordered_map<std::string, std::string>& defines)
{
    auto iModule = m_modules.find(shaderId);
    vk::ShaderModule shaderModule = nullptr;

    // Load the file if it does not exists
    if (iModule == m_modules.end()) {
        auto textCode = adaptGlslFile(shaderId, defines);
        auto type = shaderType(shaderId);
        auto code = vulkan::spvFromGlsl(type, textCode);
        auto module = vulkan::createShaderModule(m_device, code);

        auto result = m_modules.emplace(shaderId, m_device);
        if (result.second) {
            result.first->second = module;
            shaderModule = module;
        }
    }
    // Or just get it
    else {
        shaderModule = iModule->second;
    }

    return shaderModule;
}
