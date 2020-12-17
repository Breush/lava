#include "./shader.hpp"

#include <shaderc/shaderc.hpp>

#include "../wrappers.hpp"

using namespace lava::magma;
using namespace lava::chamber;

std::vector<uint32_t> vulkan::spvFromGlsl(const std::string& hrid, const std::string& source)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto module = compiler.CompileGlslToSpv(source, shaderc_glsl_infer_from_source, hrid.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        logger.warning("magma.vulkan.helpers.shader") << module.GetErrorMessage();

        std::ofstream file(".shader.tmp");
        if (file.is_open()) {
            file << source;
            file.close();
            logger.log() << "Shader code available to .shader.tmp." << std::endl;
        }

        logger.warning("magma.vulkan.helpers.shader") << "Unable to compile shader " << hrid << "." << std::endl;
        return {};
    }

    return {module.cbegin(), module.cend()};
}

vk::UniqueShaderModule vulkan::createShaderModule(vk::Device device, const std::vector<uint32_t>& code)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    auto result = device.createShaderModuleUnique(createInfo);
    return vulkan::checkMove(result, "helpers.shader", "Unable to create shader module.");
}
