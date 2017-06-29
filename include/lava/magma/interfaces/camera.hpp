#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace lava {
    class RenderEngine;
}

namespace lava {
    /**
     * Interface for cameras.
     */
    class ICamera {
    public:
        virtual ~ICamera() = default;

        virtual const glm::vec3& position() const = 0;
        virtual const glm::mat4& viewTransform() const = 0;
        virtual const glm::mat4& projectionTransform() const = 0;
    };
}
