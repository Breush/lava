#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace lava::magma {
    /**
     * Interface for lights.
     */
    class ILight {
    public:
        virtual ~ILight() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
