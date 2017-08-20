#pragma once

namespace lava::magma {
    /**
     * Interface for materials.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
