#pragma once

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for render targets.
     */
    class IRenderTarget {
    public:
        virtual ~IRenderTarget() = default;

        /// Draw.
        virtual void draw() const = 0;

        /// Refresh from size.
        virtual void refresh() = 0;
    };
}
