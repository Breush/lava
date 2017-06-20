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

        using UserData = void*;
        virtual UserData render(UserData data) = 0;
    };
}
