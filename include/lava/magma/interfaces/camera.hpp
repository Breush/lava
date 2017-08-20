#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace lava::magma {
    class RenderScene;
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

        virtual void init() = 0;

        /// Render the camera (bind it).
        virtual UserData render(UserData data) = 0;

        /// Its world position.
        virtual const glm::vec3& position() const = 0;

        /// Its view transform.
        virtual const glm::mat4& viewTransform() const = 0;

        /// Its projection transform.
        virtual const glm::mat4& projectionTransform() const = 0;
    };
}
