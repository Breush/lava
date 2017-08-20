#pragma once

#include <lava/magma/extent.hpp>

namespace lava::magma {
    /**
     * Interface for render scenes.
     */
    class IRenderScene {
    public:
        virtual ~IRenderScene() = default;

        /// Initialize the scene.
        virtual void init() = 0;

        /// Size of the rendered scene.
        virtual Extent2d extent() const = 0;
        virtual void extent(Extent2d extent) = 0;

    public:
        // @todo Do the same thing for RenderTargets,
        // and remove its UserData thingy.
        /// Pointer to the implementation.
        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
