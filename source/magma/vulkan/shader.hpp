#pragma once

#include <fstream>
#include <glslang/Public/ShaderLang.h>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

namespace lava::vulkan {
    EShLanguage findShaderLanguage(const std::string& filename)
    {
        auto extension = filename.substr(filename.find_last_of(".") + 1u);

        if (extension == "vert")
            return EShLangVertex;
        else if (extension == "tesc")
            return EShLangTessControl;
        else if (extension == "tese")
            return EShLangTessEvaluation;
        else if (extension == "geom")
            return EShLangGeometry;
        else if (extension == "frag")
            return EShLangFragment;
        else if (extension == "comp")
            return EShLangCompute;

        // @todo Error message
        return EShLangVertex;
    }

    static std::vector<uint8_t> readGlslShaderFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            logger.warning("magma.vulkan.shader") << "Unable to shader file " << filename << std::endl;
        }

        size_t fileSize = file.tellg();
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        logger.info("magma.vulkan.shader") << "Reading GLSL shader file '" << filename << "' (" << fileSize << "B)" << std::endl;

        auto compiler = ShConstructCompiler(findShaderLanguage(filename), 0);
        // @todo Error message if (compiler == 0) return;

        // CompileFile(workItem->name.c_str(), compiler);

        // if (!(Options & EOptionSuppressInfolog)) workItem->results = ShGetInfoLog(compiler);

        ShDestruct(compiler);

        return buffer;
    }

    static std::vector<uint8_t> readShaderFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            logger.warning("magma.vulkan.shader") << "Unable to shader file " << filename << std::endl;
        }

        size_t fileSize = file.tellg();
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        logger.info("magma.vulkan.shader") << "Reading shader file '" << filename << "' (" << fileSize << "B)" << std::endl;

        return buffer;
    }

    void createShaderModule(VkDevice device, const std::vector<uint8_t>& code, Capsule<VkShaderModule>& shaderModule)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();

        // We need to realigned the data as the shader module require an uint32_t array
        std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
        memcpy(codeAligned.data(), code.data(), code.size());
        createInfo.pCode = codeAligned.data();

        if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule.replace()) != VK_SUCCESS) {
            logger.error("magma.vulkan.shader") << "Failed to create shader module" << std::endl;
        }
    }
}
