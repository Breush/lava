#pragma once

namespace lava::magma {
    /**
     * Interface for meshes.
     */
    class IMesh {
    public:
        virtual ~IMesh() = default;

        /**
         * Whether the mesh is considered an opaque one.
         * Having too many non-opaque meshes can impact performances.
         *
         * @todo Move that notion to the material?
         */
        virtual bool translucent() const = 0;
        virtual void translucent(bool translucent) = 0;

        class Impl;
        virtual Impl& interfaceImpl() = 0;
    };
}
