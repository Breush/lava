#pragma once

namespace lava::magma {
    /**
     * Interface for meshes.
     */
    class IMesh {
    public:
        virtual ~IMesh() = default;

        class Impl;
        virtual Impl& interfaceImpl() = 0;

        virtual bool canCastShadows() const = 0;
        virtual void canCastShadows(bool canCastShadows) = 0;
    };
}
