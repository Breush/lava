#include "./shader.hpp"

#include <lava/chamber/string-tools.hpp>

using namespace lava;
using namespace lava::chamber;

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
        std::string spacing;
        std::string word;

        auto offset = chamber::nextWord(line, word, 0u, &spacing);

        for (; !word.empty(); offset = chamber::nextWord(line, word, offset, &spacing)) {
            // #softdefine
            if (word.find("#softdefine") != std::string::npos) {
                offset = chamber::nextWord(line, word, offset);

                const auto definePair = defines.find(word);
                if (definePair == defines.end()) {
                    logger.warning("magma.helpers.shader")
                        << "Unable to find #softdefine " << word << " correspondance while reading GLSL file " << filename << "."
                        << std::endl;
                    continue;
                }

                adaptedCode << "#define " << word << " " << definePair->second << " // #softdefine";
                continue;
            }
            // #include
            else if (word.find("#include") != std::string::npos) {
                offset = chamber::nextWord(line, word, offset);

                word = word.substr(1u, word.size() - 2u);
                auto includeFilename = filename.substr(0u, filename.find_last_of("/") + 1u) + word;
                adaptedCode << "// BEGIN #include \"" << word << "\"" << std::endl;
                adaptedCode << adaptGlslFile(includeFilename, defines) << std::endl;
                adaptedCode << "// END #include \"" << word << "\"" << std::endl;
                continue;
            }

            // Default
            adaptedCode << spacing << word;
        }
        adaptedCode << std::endl;
    }

    const auto textCode = adaptedCode.str();

    return textCode;
}
