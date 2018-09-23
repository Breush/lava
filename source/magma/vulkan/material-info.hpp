#pragma once

#include "../uniform.hpp"

#include <lava/core/filesystem.hpp>

namespace lava::magma {
    struct MaterialInfo {
        uint32_t id;
        uint32_t watchId;
        fs::Path sourcePath;
        UniformDefinitions uniformDefinitions;
    };
}
