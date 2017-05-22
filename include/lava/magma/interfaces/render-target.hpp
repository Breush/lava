#pragma once

namespace lava {
    class RenderEngine;
}

namespace lava {
    /**
     * Interface for render targets.
     */
    class IRenderTarget {
    public:
        virtual void init(RenderEngine& engine) = 0;
        virtual void draw() const = 0;
        virtual void refresh() = 0; // from size
    };
}
