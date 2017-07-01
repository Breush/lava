#pragma once

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
    /**
     * Interface for materials.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;
    };
}
