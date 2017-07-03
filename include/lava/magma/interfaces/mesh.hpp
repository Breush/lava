#pragma once

#include <glm/mat4x4.hpp>

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for meshes.
     */
    class IMesh {
    public:
        using UserData = void*;

    public:
        virtual ~IMesh() = default;

        /// Get the current world transform of the mesh.
        virtual const glm::mat4& worldTransform() const = 0; // @todo Is this a magma::Node thingy?

        /// Render the mesh.
        virtual UserData render(UserData data) = 0;
    };
}
