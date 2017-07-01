#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for point lights.
     */
    class IPointLight {
    public:
        virtual ~IPointLight() = default;

        /// World position.
        virtual const glm::vec3& position() const = 0;
    };
}
