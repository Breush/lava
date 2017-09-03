#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../helpers/shader.hpp"

namespace lava::magma::vulkan {
    /// Read a Glsl shader text code and outputs SPIR-V bytes.
    std::vector<uint8_t> spvFromGlsl(ShaderType shaderType, const std::string& textCode);

    /// Create a vk::ShaderModule from SPIR-V bytes.
    vk::ShaderModule createShaderModule(vk::Device device, const std::vector<uint8_t>& code);
}
