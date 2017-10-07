#include "./shader.hpp"

#include <SPIRV/GlslangToSpv.h>
#include <fstream>
#include <glslang/MachineIndependent/localintermediate.h>
#include <lava/chamber/logger.hpp>

namespace {
    EShLanguage findShaderLanguage(lava::magma::ShaderType shaderType)
    {
        switch (shaderType) {
        case lava::magma::ShaderType::Vertex: return EShLangVertex;
        case lava::magma::ShaderType::Fragment: return EShLangFragment;
        case lava::magma::ShaderType::TessellationControl: return EShLangTessControl;
        case lava::magma::ShaderType::TessellationEvaluation: return EShLangTessEvaluation;
        case lava::magma::ShaderType::Geometry: return EShLangGeometry;
        case lava::magma::ShaderType::Compute: return EShLangCompute;
        default: return EShLangVertex;
        }
    }

    TBuiltInResource& glslangResources()
    {
        static std::unique_ptr<TBuiltInResource> resources = nullptr;
        if (resources == nullptr) {
            resources = std::make_unique<TBuiltInResource>();
            resources->maxLights = 32;
            resources->maxClipPlanes = 6;
            resources->maxTextureUnits = 32;
            resources->maxTextureCoords = 32;
            resources->maxVertexAttribs = 64;
            resources->maxVertexUniformComponents = 4096;
            resources->maxVaryingFloats = 64;
            resources->maxVertexTextureImageUnits = 32;
            resources->maxCombinedTextureImageUnits = 80;
            resources->maxTextureImageUnits = 32;
            resources->maxFragmentUniformComponents = 4096;
            resources->maxDrawBuffers = 32;
            resources->maxVertexUniformVectors = 128;
            resources->maxVaryingVectors = 8;
            resources->maxFragmentUniformVectors = 16;
            resources->maxVertexOutputVectors = 16;
            resources->maxFragmentInputVectors = 15;
            resources->minProgramTexelOffset = -8;
            resources->maxProgramTexelOffset = 7;
            resources->maxClipDistances = 8;
            resources->maxComputeWorkGroupCountX = 65535;
            resources->maxComputeWorkGroupCountY = 65535;
            resources->maxComputeWorkGroupCountZ = 65535;
            resources->maxComputeWorkGroupSizeX = 1024;
            resources->maxComputeWorkGroupSizeY = 1024;
            resources->maxComputeWorkGroupSizeZ = 64;
            resources->maxComputeUniformComponents = 1024;
            resources->maxComputeTextureImageUnits = 16;
            resources->maxComputeImageUniforms = 8;
            resources->maxComputeAtomicCounters = 8;
            resources->maxComputeAtomicCounterBuffers = 1;
            resources->maxVaryingComponents = 60;
            resources->maxVertexOutputComponents = 64;
            resources->maxGeometryInputComponents = 64;
            resources->maxGeometryOutputComponents = 128;
            resources->maxFragmentInputComponents = 128;
            resources->maxImageUnits = 8;
            resources->maxCombinedImageUnitsAndFragmentOutputs = 8;
            resources->maxCombinedShaderOutputResources = 8;
            resources->maxImageSamples = 0;
            resources->maxVertexImageUniforms = 0;
            resources->maxTessControlImageUniforms = 0;
            resources->maxTessEvaluationImageUniforms = 0;
            resources->maxGeometryImageUniforms = 0;
            resources->maxFragmentImageUniforms = 8;
            resources->maxCombinedImageUniforms = 8;
            resources->maxGeometryTextureImageUnits = 16;
            resources->maxGeometryOutputVertices = 256;
            resources->maxGeometryTotalOutputComponents = 1024;
            resources->maxGeometryUniformComponents = 1024;
            resources->maxGeometryVaryingComponents = 64;
            resources->maxTessControlInputComponents = 128;
            resources->maxTessControlOutputComponents = 128;
            resources->maxTessControlTextureImageUnits = 16;
            resources->maxTessControlUniformComponents = 1024;
            resources->maxTessControlTotalOutputComponents = 4096;
            resources->maxTessEvaluationInputComponents = 128;
            resources->maxTessEvaluationOutputComponents = 128;
            resources->maxTessEvaluationTextureImageUnits = 16;
            resources->maxTessEvaluationUniformComponents = 1024;
            resources->maxTessPatchComponents = 120;
            resources->maxPatchVertices = 32;
            resources->maxTessGenLevel = 64;
            resources->maxViewports = 16;
            resources->maxVertexAtomicCounters = 0;
            resources->maxTessControlAtomicCounters = 0;
            resources->maxTessEvaluationAtomicCounters = 0;
            resources->maxGeometryAtomicCounters = 0;
            resources->maxFragmentAtomicCounters = 8;
            resources->maxCombinedAtomicCounters = 8;
            resources->maxAtomicCounterBindings = 1;
            resources->maxVertexAtomicCounterBuffers = 0;
            resources->maxTessControlAtomicCounterBuffers = 0;
            resources->maxTessEvaluationAtomicCounterBuffers = 0;
            resources->maxGeometryAtomicCounterBuffers = 0;
            resources->maxFragmentAtomicCounterBuffers = 1;
            resources->maxCombinedAtomicCounterBuffers = 1;
            resources->maxAtomicCounterBufferSize = 16384;
            resources->maxTransformFeedbackBuffers = 4;
            resources->maxTransformFeedbackInterleavedComponents = 64;
            resources->maxCullDistances = 8;
            resources->maxCombinedClipAndCullDistances = 8;
            resources->maxSamples = 4;
            resources->limits.nonInductiveForLoops = true;
            resources->limits.whileLoops = true;
            resources->limits.doWhileLoops = true;
            resources->limits.generalUniformIndexing = true;
            resources->limits.generalAttributeMatrixVectorIndexing = true;
            resources->limits.generalVaryingIndexing = true;
            resources->limits.generalSamplerIndexing = true;
            resources->limits.generalVariableIndexing = true;
            resources->limits.generalConstantMatrixVectorIndexing = true;
        }
        return *resources;
    }
}

using namespace lava::magma;
using namespace lava::chamber;

std::vector<uint8_t> vulkan::spvFromGlsl(ShaderType shaderType, const std::string& textCode)
{
    glslang::InitializeProcess();

    // Vulkan and SPIR-V Rules
    EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
    auto stage = findShaderLanguage(shaderType);
    auto& resources = glslangResources();
    const char* shaderString = textCode.c_str();

    // Parse file
    glslang::TShader shader(stage);
    auto errorRoutine = [&textCode, &shader]() {
        logger.warning("magma.vulkan.helpers.shader").tab(1) << shader.getInfoLog() << shader.getInfoDebugLog();

        std::ofstream file(".shader.tmp");
        if (!file.is_open()) return;

        file << textCode;
        file.close();
        logger.log() << "Shader code available to .shader.tmp." << std::endl;
    };

    shader.setStrings(&shaderString, 1);
    if (!shader.parse(&resources, 450, false, messages)) {
        errorRoutine();
        logger.error("magma.vulkan.helpers.shader").tab(-1) << "Unable to parse shader." << std::endl;
    }

    // Create program
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages)) {
        errorRoutine();
        logger.error("magma.vulkan.helpers.shader").tab(-1) << "Unable to link shader." << std::endl;
    }

    // Compile to SPIR-V
    std::vector<unsigned int> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
    if (spirv.size() == 0u) {
        errorRoutine();
        logger.error("magma.vulkan.helpers.shader").tab(-1) << "Unable to compile shader." << std::endl;
    }

    // Copy to real buffer
    std::vector<uint8_t> buffer;
    buffer.resize(spirv.size() * sizeof(unsigned int));
    memcpy(buffer.data(), spirv.data(), buffer.size());

    glslang::FinalizeProcess();

    return buffer;
}

vk::ShaderModule vulkan::createShaderModule(vk::Device device, const std::vector<uint8_t>& code)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();

    // We need to realigned the data as the shader module require an uint32_t array
    std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
    memcpy(codeAligned.data(), code.data(), code.size());
    createInfo.pCode = codeAligned.data();

    vk::ShaderModule module;
    if (device.createShaderModule(&createInfo, nullptr, &module) != vk::Result::eSuccess) {
        logger.error("magma.vulkan.helpers.shader") << "Failed to create shader module." << std::endl;
    }

    return module;
}
