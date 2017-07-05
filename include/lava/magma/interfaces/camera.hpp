#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for cameras.
     */
    class ICamera {
    public:
        using UserData = void*;

    public:
        virtual ~ICamera() = default;

        /// Render the camera (bind it).
        virtual UserData render(UserData data) = 0;
    };
}
