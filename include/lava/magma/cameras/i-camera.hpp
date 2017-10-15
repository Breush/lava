#pragma once

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

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
