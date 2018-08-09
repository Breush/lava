#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <lava/magma/render-image.hpp>

namespace lava::magma {
    /**
     * Interface for lights.
     */
    class ILight {
    public:
        virtual ~ILight() = default;

        /// The shadow map rendered image (if any).
        virtual RenderImage shadowsRenderImage() const = 0;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
