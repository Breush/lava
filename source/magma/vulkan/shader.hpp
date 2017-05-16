#pragma once

#include <fstream>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "./capsule.hpp"

namespace lava::vulkan {

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
