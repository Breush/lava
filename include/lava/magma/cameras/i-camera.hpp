#pragma once

namespace lava::magma {
    /**
     * Interface for cameras.
     */
    class ICamera {
    public:
        virtual ~ICamera() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
