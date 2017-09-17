#include "./shader.hpp"

#include <fstream>
#include <lava/chamber/logger.hpp>
#include <sstream>

using namespace lava;
using namespace lava::chamber;

magma::ShaderType magma::shaderType(const std::string& filename)
{
    static std::unordered_map<std::string, ShaderType> extensionMap = {{"vert", ShaderType::Vertex},
                                                                       {"frag", ShaderType::Fragment},
                                                                       {"tesc", ShaderType::TessellationControl},
                                                                       {"tese", ShaderType::TessellationEvaluation},
                                                                       {"geom", ShaderType::Geometry},
                                                                       {"comp", ShaderType::Compute}};

    auto extension = filename.substr(filename.find_last_of(".") + 1u);

    const auto shaderTypePair = extensionMap.find(extension);
    if (shaderTypePair == extensionMap.end()) {
        return ShaderType::Unknown;
    }

    return shaderTypePair->second;
}

std::string magma::adaptGlslFile(const std::string& filename, const std::unordered_map<std::string, std::string>& defines)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        logger.warning("magma.helpers.shader") << "Unable to find shader file " << filename << "." << std::endl;
        return "";
    }

    // Adapting the code
    std::stringstream adaptedCode;
    std::string line;

    while (std::getline(file, line)) {
        std::string word;
        std::istringstream lineStream(line);
        while (lineStream >> word) {
            // #softdefine
            if (word.find("#softdefine") != std::string::npos) {
                lineStream >> word;

                const auto definePair = defines.find(word);
                if (definePair == defines.end()) {
                    logger.warning("magma.helpers.shader")
                        << "Unable to find #softdefine " << word << " correspondance while reading GLSL file " << filename << "."
                        << std::endl;
                    continue;
                }

                adaptedCode << "// #softdefine " << word << std::endl;
                adaptedCode << "#define " << word << " " << definePair->second << " ";
                continue;
            }
            // #include
            else if (word.find("#include") != std::string::npos) {
                lineStream >> word;

                word = word.substr(1u, word.size() - 2u);
                auto includeFilename = filename.substr(0u, filename.find_last_of("/") + 1u) + word;
                adaptedCode << "// BEGIN #include \"" << word << "\"" << std::endl;
                adaptedCode << adaptGlslFile(includeFilename, defines) << std::endl;
                adaptedCode << "// END #include \"" << word << "\"" << std::endl;
                continue;
            }

            // Default
            adaptedCode << word << " ";
        }
        adaptedCode << std::endl;
    }

    const auto textCode = adaptedCode.str();

    return textCode;
}
