#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace lava::magma {
    enum class UniformType {
        Unknown,
        Bool,
        Uint,
        Float,
        Vec2,
        Vec3,
        Vec4,
        Texture,
        CubeTexture,
    };

    enum class UniformTextureType {
        Unknown,
        White,
        Normal,
        Invisible,
    };

    union UniformFallback {
        uint32_t uintValue;
        float floatValue;
        glm::vec2 vec2Value;
        glm::vec3 vec3Value;
        glm::vec4 vec4Value;
        UniformTextureType textureTypeValue;
        uint32_t uintArrayValue[64];

        UniformFallback() {}
        UniformFallback(uint32_t value) { uintValue = value; }
        UniformFallback(float value) { floatValue = value; }
        UniformFallback(const glm::vec2& value) { vec2Value = value; }
        UniformFallback(const glm::vec3& value) { vec3Value = value; }
        UniformFallback(const glm::vec4& value) { vec4Value = value; }
        UniformFallback(UniformTextureType value) { textureTypeValue = value; }
        UniformFallback(const uint32_t* values, uint32_t size)
        {
            for (auto i = 0u; i < size; ++i) {
                uintArrayValue[i] = values[i];
            }
        }
    };

    struct UniformDefinition {
        std::string name;
        UniformType type = UniformType::Unknown;
        UniformFallback fallback;
        uint32_t arraySize = 0u; // 0u means not an array.
        uint32_t offset = 0u;
    };

    using UniformDefinitions = std::vector<UniformDefinition>;
}
