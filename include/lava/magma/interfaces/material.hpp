#pragma once

namespace lava {
    class RenderEngine;
}

namespace lava {
    /**
     * Interface for render materials.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        virtual void init(RenderEngine& engine) = 0;
    };
}
