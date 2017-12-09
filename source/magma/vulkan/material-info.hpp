#pragma once

#include <lava/magma/uniform.hpp>

#include <cstdint>
#include <string>

namespace lava::magma {
    struct MaterialInfo {
        uint32_t id;
        std::string sourcePath;
        UniformDefinitions uniformDefinitions;
    };
}
