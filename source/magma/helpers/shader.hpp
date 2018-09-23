#pragma once

namespace lava::magma {
    /**
     * Return the text of glsl file with all defines resolved.
     *
     * The defines list will change any line like:
     *      #softdefine WHATEVER
     * to:
     *      #define WHATEVER <valueSpecified>
     */
    std::string adaptGlslFile(const std::string& filename, const std::unordered_map<std::string, std::string>& defines);
}
