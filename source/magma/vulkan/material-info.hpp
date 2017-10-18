#pragma once

#include <lava/magma/uniform.hpp>

#include <cstdint>

namespace lava::magma {
    struct MaterialInfo {
        uint32_t id;
        UniformDefinitions uniformDefinitions;
    };
}
