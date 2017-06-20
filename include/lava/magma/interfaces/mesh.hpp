#pragma once

namespace lava {
    class RenderEngine;
}

namespace lava {
    /**
     * Interface for meshes.
     */
    class IMesh {
    public:
        virtual ~IMesh() = default;

        virtual void* render(void* data) = 0;
    };
}
