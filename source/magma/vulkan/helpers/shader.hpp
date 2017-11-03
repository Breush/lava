#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "../../helpers/shader.hpp"

namespace lava::magma::vulkan {
    /// Read a Glsl shader text code and outputs SPIR-V bytes.
    std::vector<uint32_t> spvFromGlsl(const std::string& hrid, const std::string& source);

    /// Create a vk::ShaderModule from SPIR-V bytes.
    vk::ShaderModule createShaderModule(vk::Device device, const std::vector<uint32_t>& code);
}
