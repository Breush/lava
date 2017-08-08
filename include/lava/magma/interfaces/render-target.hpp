#pragma once

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for render targets.
     */
    class IRenderTarget {
    protected:
        using UserData = const void*;

    public:
        virtual ~IRenderTarget() = default;

        /// Called once when added to the engine.
        virtual void init(UserData data) = 0;

        /// Prepare the upcoming draw.
        virtual void prepare() = 0;

        /// Draw.
        virtual void draw(UserData data) const = 0;

        /// Get some internal data.
        virtual UserData data() = 0;
    };
}
