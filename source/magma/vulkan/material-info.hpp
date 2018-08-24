#pragma once

#include "../uniform.hpp"

#include <cstdint>
#include <lava/core/filesystem.hpp>
#include <string>

namespace lava::magma {
    struct MaterialInfo {
        uint32_t id;
        uint32_t watchId;
        fs::Path sourcePath;
        UniformDefinitions uniformDefinitions;
    };
}
