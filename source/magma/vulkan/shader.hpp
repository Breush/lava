#pragma once

#include <fstream>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace lava::magma::vulkan {
    /// Read a Glsl shader file and outputs SPIR-V bytes.
    std::vector<uint8_t> readGlslShaderFile(const std::string& filename);

    /// Create a vk::ShaderModule from SPIR-V bytes.
    vk::ShaderModule createShaderModule(vk::Device device, const std::vector<uint8_t>& code);
}
