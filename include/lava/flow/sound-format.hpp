#pragma once

#include <lava/core/macros/enum.hpp>

/**
 * Defines how the sample info is stored.
 */
$enum_class(lava::flow, SoundFormat,
            Unknown, // Unknown
            Uint8,   // Unsigned 8 bits integer
            Int16LE, // Signed 16 bits integer, little endian
            Int16BE, // Signed 16 bits integer, big endian
            Int32LE, // Signed 32 bits integer, little endian
            Int32BE, // Signed 32 bits integer, big endian
);
