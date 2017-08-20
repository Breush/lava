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
        using UserData = void*;

    public:
        virtual ~IMaterial() = default;

        virtual void init() = 0;

        /// Render the material (bind it).
        virtual UserData render(UserData data) = 0;
    };
}
