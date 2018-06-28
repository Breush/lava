#pragma once

#include <cstdint>

namespace lava::chamber {
    /**
     * Tool functions to extract data from raw memory.
     */

    inline uint16_t readUint16LE(const uint8_t* buffer, uint32_t offset);
    inline uint32_t readUint32LE(const uint8_t* buffer, uint32_t offset);
}

#include <lava/chamber/buffer.inl>
