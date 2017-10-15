#pragma once

namespace lava::glb {
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

    template <class T>
    inline std::vector<T> Accessor::get(const typename nlohmann::json::basic_json& bufferViews,
                                        const std::vector<uint8_t>& buffer) const
    {
        uint32_t bufferViewIndex = bufferView;
        BufferView bufferView(bufferViews[bufferViewIndex]);
        std::vector<T> vector(count);

        auto offset = byteOffset + bufferView.byteOffset;

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
