#include "./shaders-manager.hpp"

#include "../helpers/shader.hpp"
#include "./helpers/shader.hpp"

using namespace lava::magma;
using namespace lava::chamber;

ShadersManager::ShadersManager(const vk::Device& device)
    : m_device(device)
{
}

void ShadersManager::update()
{
    for (const auto& category : m_dirtyCategories) {
        for (const auto& updateCallback : m_impls[category].updateCallbacks) {
            updateCallback();
        }
    }
    m_dirtyCategories.clear();
}


void ShadersManager::registerImplGroup(const std::string& hrid, const std::string& rawCode, uint32_t implsId)
{
    auto& implGroup = m_implGroups[hrid];

    auto impls = extractImpls(rawCode);
    for (const auto& impl : impls) {
        const auto implId = registerImpl(impl.first, impl.second, implsId);
        implGroup.implIds[impl.first] = implId;
    }
}

void ShadersManager::updateImplGroup(const std::string& hrid, const std::string& rawCode)
{
    auto& implGroup = m_implGroups.at(hrid);

    auto impls = extractImpls(rawCode);
    for (const auto& impl : impls) {
        const auto implId = implGroup.implIds.at(impl.first);
        updateImpl(impl.first, implId, impl.second);
    }
}

vk::ShaderModule ShadersManager::module(const std::string& shaderId)
{
    ModuleOptions options;
    return module(shaderId, options);
}

vk::ShaderModule ShadersManager::module(const std::string& shaderId, const ModuleOptions& options)
{
    auto iModuleInfo = m_modulesInfos.find(shaderId);
    auto isModuleDirty = m_dirtyModules.find(shaderId) != m_dirtyModules.end();

    vk::ShaderModule shaderModule = nullptr;
    std::set<std::string> implsDependencies;

    // Load the file if it does not exists or if it is dirty
    if (iModuleInfo == m_modulesInfos.end() || isModuleDirty) {
        auto textCode = adaptGlslFile(shaderId, options.defines);
        auto resolvedShader = resolveShader(textCode);

        logger.info("magma.vulkan.shaders-manager")
            << "Reading GLSL shader file '" << shaderId << "' (" << resolvedShader.textCode.size() << "B)." << std::endl;

        auto code = vulkan::spvFromGlsl(shaderId, resolvedShader.textCode);
        if (code.empty()) {
            // We were not able to compule GLSL file, we try to use previously existing shader
            // if possible.
            if (iModuleInfo != m_modulesInfos.end()) {
                return iModuleInfo->second.module.get();
            }
            else {
                logger.error("magma.vulkan.shaders-manager") << "No valid shader for " << shaderId << "." << std::endl;
            }
            return nullptr;
        }

        // Destroy previous module if any
        if (isModuleDirty && iModuleInfo != m_modulesInfos.end()) {
            m_modulesInfos.erase(iModuleInfo);
        }

        auto newShaderModule = vulkan::createShaderModule(m_device, code);
        shaderModule = newShaderModule.get();
        implsDependencies = resolvedShader.implsDependencies;

        // Adding the module
        auto& newModuleInfo = m_modulesInfos[shaderId];
        newModuleInfo.module = std::move(newShaderModule);
        newModuleInfo.implsDependencies = implsDependencies;

        m_dirtyModules.erase(shaderId);
    }
    // Or just get it
    else {
        shaderModule = iModuleInfo->second.module.get();
        implsDependencies = iModuleInfo->second.implsDependencies;
    }

    // Adding the callback to all of his dependencies
    for (const auto& category : implsDependencies) {
        if (options.updateCallback != nullptr) {
            m_impls[category].updateCallbacks.emplace_back(options.updateCallback);
        }
        m_impls[category].dirtyShaderIds.emplace(shaderId);
    }

    return shaderModule;
}

//----- Internal

void ShadersManager::dirtifyImpl(const std::string& category)
{
    m_dirtyCategories.emplace(category);

    // Remove modules that need to be updated
    for (const auto& shaderId : m_impls[category].dirtyShaderIds) {
        m_dirtyModules.emplace(shaderId);
    }
}

uint32_t ShadersManager::registerImpl(const std::string& category, const std::string& implCode, uint32_t implId)
{
    auto resolvedShader = resolveImpl(category, implId, implCode);
    m_impls[category].textCodes.emplace(implId, resolvedShader.textCode);

    dirtifyImpl(category);

    return implId;
}

void ShadersManager::updateImpl(const std::string& category, const uint32_t implId, const std::string& implCode)
{
    auto resolvedShader = resolveImpl(category, implId, implCode);

    if (m_impls[category].textCodes[implId] != resolvedShader.textCode) {
        m_impls[category].textCodes[implId] = resolvedShader.textCode;
        dirtifyImpl(category);
    }
}

ShadersManager::ResolvedShader ShadersManager::resolveImpl(const std::string& category, const uint32_t implId,
                                                           const std::string& implCode)
{
    std::stringstream implTitle;
    implTitle << category << implId;

    return resolveShader(implCode, implTitle.str());
}

ShadersManager::ResolvedShader ShadersManager::resolveShader(const std::string& textCode, const std::string& annotationMain)
{
    std::stringstream textCodeStream(textCode);
    std::stringstream adaptedCode;
    std::string line;

    std::string category;
    std::string casesBuffer;
    std::string casesSpacing;
    bool casesBuffering = false;

    std::set<std::string> implsDependencies;

    while (std::getline(textCodeStream, line)) {
        std::string spacing;
        std::string word;
        auto offset = chamber::nextWord(line, word, 0u, &spacing);

        for (; !word.empty(); offset = chamber::nextWord(line, word, offset, &spacing)) {
            // @magma:impl:paste
            if (word.find("@magma:impl:paste") != std::string::npos) {
                offset = chamber::nextWord(line, category, offset);
                adaptedCode << "// BEGIN @magma:impl:paste " << category << std::endl;
                for (const auto& implTextCode : m_impls[category].textCodes) {
                    adaptedCode << implTextCode.second << std::endl;
                }
                adaptedCode << "// END @magma:impl:paste " << category << std::endl;
                implsDependencies.emplace(category);
                continue;
            }
            // @magma:impl:main
            else if (word.find("@magma:impl:main") != std::string::npos) {
                adaptedCode << spacing << annotationMain;
                continue;
            }
            // @magma:impl:beginCases
            else if (word.find("@magma:impl:beginCases") != std::string::npos) {
                casesSpacing = spacing;
                offset = chamber::nextWord(line, category, offset);
                adaptedCode << spacing << "// BEGIN @magma:impl:beginCases " << category << std::endl;
                casesBuffering = true;
                casesBuffer.clear();
                implsDependencies.emplace(category);
                continue;
            }
            // @magma:impl:endCases
            else if (word.find("@magma:impl:endCases") != std::string::npos) {
                static std::string callMark = "@magma:impl:call";
                auto callMarkPos = casesBuffer.find(callMark);
                auto callMarkSize = callMark.size();

                for (const auto& implTextCode : m_impls[category].textCodes) {
                    std::stringstream callStream;
                    callStream << category << implTextCode.first;
                    auto callString = callStream.str();
                    auto caseBuffer = casesBuffer.replace(callMarkPos, callMarkSize, callString);
                    callMarkSize = callString.size();

                    adaptedCode << casesSpacing << "case " << implTextCode.first << ":" << caseBuffer;
                    adaptedCode << casesSpacing << "break;" << std::endl;
                }

                adaptedCode << casesSpacing << "default: break;" << std::endl;
                adaptedCode << casesSpacing << "// END @magma:impl:endCases " << category;
                casesBuffering = false;
                continue;
            }

            if (casesBuffering) {
                casesBuffer += spacing + word;
                continue;
            }

            // Default
            adaptedCode << spacing << word;
        }

        if (casesBuffering) {
            casesBuffer += '\n';
            continue;
        }

        adaptedCode << std::endl;
    }

    ResolvedShader resolvedShader;
    resolvedShader.textCode = adaptedCode.str();
    resolvedShader.implsDependencies = implsDependencies;

    return resolvedShader;
}

std::vector<std::pair<std::string, std::string>> ShadersManager::extractImpls(const std::string& rawCode)
{
    std::vector<std::pair<std::string, std::string>> impls;
    std::unique_ptr<std::stringstream> implCode = nullptr;
    std::string category;

    std::stringstream rawCodeStream(rawCode);
    std::string line;

    while (std::getline(rawCodeStream, line)) {
        std::string spacing;
        std::string word;
        uint32_t offset = chamber::nextWord(line, word, 0u, &spacing);

        for (; !word.empty(); offset = chamber::nextWord(line, word, offset, &spacing)) {
            // @magma:impl:begin
            if (word.find("@magma:impl:begin") != std::string::npos) {
                offset = chamber::nextWord(line, category, offset);
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
                (*implCode) << spacing << word;
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
