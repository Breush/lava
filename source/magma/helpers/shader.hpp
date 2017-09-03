#pragma once

#include <string>
#include <unordered_map>

namespace lava::magma {
    /**
     * All valid types of shaders.
     */
    enum class ShaderType { Unknown, Vertex, Fragment, TessellationControl, TessellationEvaluation, Geometry, Compute };

    /**
     * Find the shader type associated to a filename - given its extension.
     */
    ShaderType shaderType(const std::string& filename);

    /**
     * Return the text of glsl file with all defines resolved.
     *
     * The defines list will change any line like:
     *      #lava:define WHATEVER
     * to:
     *      #define WHATEVER <valueSpecified>
     */
    std::string adaptGlslFile(const std::string& filename, const std::unordered_map<std::string, std::string>& defines);
}
