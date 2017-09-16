#include "./shaders-manager.hpp"

#include "../helpers/shader.hpp"
#include "./helpers/shader.hpp"

#include <iostream>
#include <lava/chamber/logger.hpp>

using namespace lava::magma;
using namespace lava::chamber;

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
        logger.info("magma.vulkan.shaders-manager")
            << "Reading GLSL shader file '" << shaderId << "' (" << textCode.size() << "B)." << std::endl;

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
