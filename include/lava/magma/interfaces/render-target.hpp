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

        virtual void init(RenderEngine& engine) = 0;
        virtual void draw() const = 0;
        virtual void refresh() = 0; // from size
    };
}
