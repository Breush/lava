#include "./shaders-manager.hpp"

#include "../helpers/shader.hpp"
#include "./helpers/shader.hpp"

#include <lava/chamber/logger.hpp>
#include <sstream>

using namespace lava::magma;
using namespace lava::chamber;

ShadersManager::ShadersManager(const vk::Device& device)
    : m_device(device)
{
}

uint32_t ShadersManager::registerImpl(const std::string& category, const std::string& implCode)
{
    const uint32_t id = m_impls[category].size();
    std::stringstream implTitle;
    implTitle << category << id;
    m_impls[category].emplace_back(resolve(implCode, implTitle.str()));
    return id;
}

void ShadersManager::registerImpls(const std::string& rawCode)
{
    auto impls = extractImpls(rawCode);
    for (const auto& impl : impls) {
        registerImpl(impl.first, impl.second);
    }
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
        textCode = resolve(textCode);
        logger.info("magma.vulkan.shaders-manager")
            << "Reading GLSL shader file '" << shaderId << "' (" << textCode.size() << "B)." << std::endl;

        auto code = vulkan::spvFromGlsl(shaderId, textCode);
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

//----- Internal

std::string ShadersManager::resolve(const std::string& textCode, std::string annotationMain)
{
    std::stringstream textCodeStream(textCode);
    std::stringstream adaptedCode;
    std::string line;

    std::string category;
    std::string casesBuffer;
    bool casesBuffering = false;

    while (std::getline(textCodeStream, line)) {
        std::string word;
        std::istringstream lineStream(line);
        while (lineStream >> word) {
            // @magma:impl:paste
            if (word.find("@magma:impl:paste") != std::string::npos) {
                lineStream >> category;
                adaptedCode << "// BEGIN @magma:impl:paste " << category << std::endl;
                for (const auto& impl : m_impls[category]) {
                    adaptedCode << impl << std::endl;
                }
                adaptedCode << "// END @magma:impl:paste " << category << std::endl;
                continue;
            }
            // @magma:impl:main
            else if (word.find("@magma:impl:main") != std::string::npos) {
                adaptedCode << annotationMain << " ";
                continue;
            }
            // @magma:impl:beginCases
            else if (word.find("@magma:impl:beginCases") != std::string::npos) {
                lineStream >> category;
                adaptedCode << "// BEGIN @magma:impl:beginCases " << category << std::endl;
                casesBuffering = true;
                casesBuffer = "";
                continue;
            }
            // @magma:impl:endCases
            else if (word.find("@magma:impl:endCases") != std::string::npos) {
                std::string callMark = "@magma:impl:call";
                auto callMarkPos = casesBuffer.find(callMark);
                auto callMarkSize = callMark.size();

                for (uint32_t i = 0; i < m_impls[category].size(); ++i) {
                    std::stringstream callStream;
                    callStream << category << i;
                    auto callString = callStream.str();
                    auto caseBuffer = casesBuffer.replace(callMarkPos, callMarkSize, callString);
                    callMarkSize = callString.size();

                    adaptedCode << "case " << i << ":" << std::endl;
                    adaptedCode << caseBuffer << std::endl;
                    adaptedCode << "break;" << std::endl;
                }

                adaptedCode << "default: break;" << std::endl;
                adaptedCode << "// END @magma:impl:endCases " << category << std::endl;
                casesBuffering = false;
                continue;
            }

            if (casesBuffering) {
                casesBuffer += word + " ";
                continue;
            }

            // Default
            adaptedCode << word << " ";
        }

        if (casesBuffering) {
            casesBuffer += "\n";
            continue;
        }

        adaptedCode << std::endl;
    }

    return adaptedCode.str();
}

std::vector<std::pair<std::string, std::string>> ShadersManager::extractImpls(const std::string& rawCode)
{
    std::vector<std::pair<std::string, std::string>> impls;
    std::unique_ptr<std::stringstream> implCode = nullptr;
    std::string category;

    std::stringstream rawCodeStream(rawCode);
    std::string line;

    while (std::getline(rawCodeStream, line)) {
        std::string word;
        std::istringstream lineStream(line);
        while (lineStream >> word) {
            // @magma:impl:begin
            if (word.find("@magma:impl:begin") != std::string::npos) {
                lineStream >> category;
                implCode = std::make_unique<std::stringstream>();
                continue;
            }
            // @magma:impl:end
            else if (word.find("@magma:impl:end") != std::string::npos) {
                impls.emplace_back(std::make_pair(category, implCode->str()));
                implCode = nullptr;
                continue;
            }

            if (implCode) {
                (*implCode) << word << " ";
            }
        }

        if (implCode) {
            (*implCode) << std::endl;
        }
    }

    if (implCode) {
        logger.error("magma.vulkan.shaders-manager")
            << "Extracting impl has a @magma:begin without a corresponding @magma:end." << std::endl;
    }

    return impls;
}
