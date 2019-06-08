#pragma once

#include <lava/magma/polygon-mode.hpp>
#include <lava/magma/render-image.hpp>

#include <lava/core/extent.hpp>

namespace lava::magma {
    /**
     * Interface for cameras.
     */
    class ICamera {
    public:
        virtual ~ICamera() = default;

        /// The rendered size.
        virtual Extent2d extent() const = 0;
        virtual void extent(Extent2d extent) = 0;

        /// The rendered images.
        virtual RenderImage renderImage() const = 0;
        virtual RenderImage depthRenderImage() const = 0;

        // Rendering aspect, choose Polygon::Line for wireframe.
        virtual PolygonMode polygonMode() const = 0;
        virtual void polygonMode(PolygonMode polygonMode) = 0;

        // Rendering range
        virtual float nearClip() const = 0;
        virtual void nearClip(float nearClip) = 0;
        virtual float farClip() const = 0;
        virtual void farClip(float farClip) = 0;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
        virtual const Impl& interfaceImpl() const = 0;
    };
}
