#pragma once

#include <glm/vec4.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    enum class UniformType {
        FLOAT,
        VEC4,
        TEXTURE,
        UNKNOWN,
    };

    enum class UniformTextureType {
        WHITE,
        NORMAL,
        INVISIBLE,
        UNKNOWN,
    };

    union UniformFallback {
        float floatValue;
        glm::vec4 vec4Value;
        UniformTextureType textureTypeValue;

        UniformFallback() {}
        UniformFallback(float value) { floatValue = value; }
        UniformFallback(const glm::vec4& value) { vec4Value = value; }
        UniformFallback(UniformTextureType value) { textureTypeValue = value; }
    };

    struct UniformDefinition {
        std::string name;
        UniformType type = UniformType::UNKNOWN;
        UniformFallback fallback;
    };

    using UniformDefinitions = std::vector<UniformDefinition>;
}
