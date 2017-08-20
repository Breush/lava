#pragma once

#include <lava/magma/extent.hpp>

namespace lava::magma {
    /**
     * Interface for render scenes.
     */
    class IRenderScene {
    public:
        virtual ~IRenderScene() = default;

        /// Size of the rendered scene.
        virtual Extent2d extent() const = 0;
        virtual void extent(Extent2d extent) = 0;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
