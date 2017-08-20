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
    // @todo IPointLight, really?
    // What two different classes would derive?
    class IPointLight {
    public:
        virtual ~IPointLight() = default;

        virtual void init() = 0;

        /// World position.
        virtual const glm::vec3& position() const = 0;

        /// Radius.
        virtual float radius() const = 0;
    };
}
