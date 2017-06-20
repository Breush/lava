#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

namespace lava::glb {
    struct Header {
        uint8_t magic[4];
        uint32_t version;
        uint32_t length = 0u;
    };

    inline std::ostream& operator<<(std::ostream& os, const Header& header);
    inline std::istream& operator>>(std::istream& is, Header& header);

    struct Chunk {
        enum class Type {
            JSON,
            BIN,
            UNKNOWN,
        };

        uint32_t length = 0u;
        Type type = Type::UNKNOWN;
        std::vector<uint8_t> data;
    };

    inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
    inline std::istream& operator>>(std::istream& is, Chunk& chunk);

    struct Image {
        uint32_t bufferView = -1u;
        std::string mimeType;

        Image(const typename nlohmann::json::basic_json& json);
    };

    struct Texture {
        uint32_t sampler = -1u;
        uint32_t source = -1u;

        Texture(const typename nlohmann::json::basic_json& json);
    };

    struct PbrMetallicRoughnessMaterial {
        uint32_t baseColorTextureIndex = -1u;

        PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json);
    };

    struct Accessor {
        uint32_t bufferView = -1u;
        uint32_t byteOffset = 0u;
        uint32_t count = 0u;

        Accessor(const typename nlohmann::json::basic_json& json);

        template <class T>
        std::vector<T> get(const typename nlohmann::json::basic_json& bufferViews, const std::vector<uint8_t>& buffer) const;
    };

    struct BufferView {
        uint32_t byteLength = 0u;
        uint32_t byteOffset = 0u;
        uint32_t byteStride = 0u;

        BufferView(const typename nlohmann::json::basic_json& json);

        std::vector<uint8_t> get(const std::vector<uint8_t>& buffer) const;
    };
}

#include "./loader.inl"
