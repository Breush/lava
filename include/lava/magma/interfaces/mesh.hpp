#pragma once

namespace lava::magma {
    class RenderEngine;
}

namespace lava::magma {
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
