#pragma once

#include <fstream>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace lava {
    struct Header {
        uint8_t magic[4];
        uint32_t version;
        uint32_t length = 0u;
    };

    inline std::ostream& operator<<(std::ostream& os, const Header& header)
    {
        os << "magic: " << header.magic[0] << header.magic[1] << header.magic[2] << header.magic[3] << std::endl;
        os << "version: " << header.version << std::endl;
        os << "length: " << header.length;
        return os;
    }

    inline std::istream& operator>>(std::istream& is, Header& header)
    {
        // @todo There can be a problem on big endian platforms
        is.read(reinterpret_cast<char*>(&header), 12);
        return is;
    }

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

    inline std::ostream& operator<<(std::ostream& os, const Chunk& chunk)
    {
        os << "length: " << chunk.length << std::endl;
        os << "type: ";
        switch (chunk.type) {
        case Chunk::Type::JSON: os << "JSON"; break;
        case Chunk::Type::BIN: os << "BIN"; break;
        case Chunk::Type::UNKNOWN: os << "UNKNOWN"; break;
        }
        return os;
    }

    inline std::istream& operator>>(std::istream& is, Chunk& chunk)
    {
        is.read(reinterpret_cast<char*>(&chunk.length), 4);

        uint32_t type;
        is.read(reinterpret_cast<char*>(&type), 4);
        if (type == 0x4E4F534A) {
            chunk.type = Chunk::Type::JSON;
        }
        else if (type == 0x004E4942) {
            chunk.type = Chunk::Type::BIN;
        }

        chunk.data.resize(chunk.length);
        is.read(reinterpret_cast<char*>(chunk.data.data()), chunk.length);
        return is;
    }

    struct Image {
        uint32_t bufferView = -1u;
        std::string mimeType;

        Image(const typename nlohmann::json::basic_json& json)
        {
            bufferView = json["bufferView"];
            mimeType = json["mimeType"];
        }
    };

    struct Texture {
        uint32_t sampler = -1u;
        uint32_t source = -1u;

        Texture(const typename nlohmann::json::basic_json& json)
        {
            sampler = json["sampler"];
            source = json["source"];
        }
    };

    struct PbrMetallicRoughnessMaterial {
        uint32_t baseColorTextureIndex = -1u;

        PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json)
        {
            baseColorTextureIndex = json["pbrMetallicRoughness"]["baseColorTexture"]["index"];
        }
    };

    struct Accessor {
        uint32_t bufferView = 0u;
        uint32_t byteOffset = 0u;
        uint32_t count = 0u;

        Accessor(const typename nlohmann::json::basic_json& json)
        {
            bufferView = json["bufferView"];
            byteOffset = json["byteOffset"];
            count = json["count"];
        }
    };

    struct BufferView {
        uint32_t byteLength = 0u;
        uint32_t byteOffset = 0u;
        uint32_t byteStride = 0u;

        BufferView(const typename nlohmann::json::basic_json& json)
        {
            byteLength = json["byteLength"];
            byteOffset = json["byteOffset"];
            if (json.find("byteStride") != json.end()) byteStride = json["byteStride"];
        }
    };

    inline std::vector<uint8_t> access(const BufferView& bufferView, const std::vector<uint8_t>& buffer)
    {
        std::vector<uint8_t> vector(bufferView.byteLength);
        memcpy(vector.data(), &buffer[bufferView.byteOffset], vector.size());
        return vector;
    }

    template <class T>
    inline std::vector<T> access(const Accessor& accessor, const typename nlohmann::json::basic_json& bufferViews,
                                 const std::vector<uint8_t>& buffer)
    {
        uint32_t bufferViewIndex = accessor.bufferView;
        BufferView bufferView(bufferViews[bufferViewIndex]);
        std::vector<T> vector(accessor.count);

        auto offset = accessor.byteOffset + bufferView.byteOffset;

        auto stride = bufferView.byteStride;
        if (stride == 0u) stride = sizeof(T);

        // Continuous memory, optimisation
        if (stride == sizeof(T)) {
            memcpy(vector.data(), &buffer[offset], vector.size() * sizeof(T));
        }
        else {
            for (auto i = 0u; i < vector.size(); ++i) {
                memcpy(&vector[i], &buffer[offset], sizeof(T));
                offset += stride;
            }
        }

        return vector;
    }
}
