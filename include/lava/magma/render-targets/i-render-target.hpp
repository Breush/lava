#pragma once

namespace lava::magma {
    /**
     * Interface for render targets.
     */
    class IRenderTarget {
    public:
        virtual ~IRenderTarget() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
