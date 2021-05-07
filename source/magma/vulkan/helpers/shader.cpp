#include "./shader.hpp"

#include <shaderc/shaderc.hpp>

#include "../wrappers.hpp"

using namespace lava::magma;
using namespace lava::chamber;

std::vector<uint32_t> vulkan::spvFromGlsl(const std::string& hrid, const std::string& source)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    options.SetTargetSpirv(shaderc_spirv_version_1_5);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    // Try to find the shader kind from it's extension
    shaderc_shader_kind shaderKind = shaderc_glsl_infer_from_source;
    if (hrid.size() >= 5) {
        auto extension = std::string_view(hrid).substr(hrid.size() - 5);
        if (extension == ".rgen") {
            shaderKind = shaderc_raygen_shader;
        }
        else if (extension == "rmiss") {
            shaderKind = shaderc_miss_shader;
        }
        else if (extension == "rchit") {
            shaderKind = shaderc_closesthit_shader;
        }
    }

    auto module = compiler.CompileGlslToSpv(source, shaderKind, hrid.c_str(), options);

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

    static int i = 0;
    ++i;
    std::ofstream fout;
    fout.open("./data/tmp/" + std::filesystem::path(hrid).filename().string() + ".spv", std::ios::binary | std::ios::out);
    fout.write(reinterpret_cast<const char*>(module.cbegin()), uint64_t(module.cend() - module.cbegin()) * 4u);
    fout.close();

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
