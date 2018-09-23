#pragma once

namespace lava::magma {
    enum class UniformType {
        Unknown,
        Float,
        Vec4,
        Texture,
    };

    enum class UniformTextureType {
        Unknown,
        White,
        Normal,
        Invisible,
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
        UniformType type = UniformType::Unknown;
        UniformFallback fallback;
    };

    using UniformDefinitions = std::vector<UniformDefinition>;
}
