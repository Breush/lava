#pragma once

#include "../../helpers/shader.hpp"

namespace lava::magma::vulkan {
    /// Read a Glsl shader text code and outputs SPIR-V bytes.
    std::vector<uint32_t> spvFromGlsl(const std::string& hrid, const std::string& source);

    /// Create a vk::UniqueShaderModule from SPIR-V bytes.
    vk::UniqueShaderModule createShaderModule(vk::Device device, const std::vector<uint32_t>& code);
}
