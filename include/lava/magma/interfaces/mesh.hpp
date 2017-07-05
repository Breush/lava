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

        /// Render the mesh.
        virtual UserData render(UserData data) = 0;
    };
}
