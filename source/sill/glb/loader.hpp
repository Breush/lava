#pragma once

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

    struct Node {
        std::string name;
        uint32_t meshIndex = -1u;
        std::vector<uint32_t> children;
        glm::mat4 transform;

        Node(const typename nlohmann::json::basic_json& json);
    };

    struct Mesh {
        struct Primitive {
            uint8_t mode = 4u;
            uint32_t positionsAccessorIndex = -1u;
            uint32_t normalsAccessorIndex = -1u;
            uint32_t tangentsAccessorIndex = -1u;
            uint32_t uv1sAccessorIndex = -1u;
            uint32_t indicesAccessorIndex = -1u;
            uint32_t materialIndex = -1u;
        };

        std::string name;
        std::vector<Primitive> primitives;

        Mesh(const typename nlohmann::json::basic_json& json);
    };

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

    struct Material {
        uint32_t normalTextureIndex = -1u;
        uint32_t occlusionTextureIndex = -1u;
        uint32_t emissiveTextureIndex = -1u;

        Material(const typename nlohmann::json::basic_json& json);
    };

    struct PbrMetallicRoughnessMaterial : public Material {
        uint32_t baseColorTextureIndex = -1u;
        uint32_t metallicRoughnessTextureIndex = -1u;

        PbrMetallicRoughnessMaterial(const typename nlohmann::json::basic_json& json);
    };

    struct Accessor {
        uint32_t bufferView = -1u;
        uint32_t byteOffset = 0u;
        uint32_t count = 0u;

        Accessor(const typename nlohmann::json::basic_json& json);

        template <class T>
        VectorView<T> get(const typename nlohmann::json::basic_json& bufferViews, const std::vector<uint8_t>& buffer) const;
    };

    struct BufferView {
        uint32_t byteLength = 0u;
        uint32_t byteOffset = 0u;
        uint32_t byteStride = 0u;

        BufferView(const typename nlohmann::json::basic_json& json);

        struct Data {
            const uint8_t* data = nullptr;
            uint32_t size = 0u;
        };

        VectorView<uint8_t> get(const std::vector<uint8_t>& buffer) const;
    };
}

#include "./loader.inl"
