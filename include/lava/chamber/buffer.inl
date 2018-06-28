#include <cstdint>

namespace lava::chamber {
    inline uint16_t readUint16LE(const uint8_t* buffer, uint32_t offset)
    {
        uint16_t value = (buffer[offset + 1u] << 8u) + buffer[offset + 0u];
        return value;
    }

    inline uint32_t readUint32LE(const uint8_t* buffer, uint32_t offset)
    {
        uint32_t value =
            (buffer[offset + 3u] << 24u) + (buffer[offset + 2u] << 16u) + (buffer[offset + 1u] << 8u) + buffer[offset + 0u];
        return value;
    }
}
