#pragma once

#include <fstream>

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
        os << "length: " << header.length << std::endl;
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
        os << std::endl;
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
}
