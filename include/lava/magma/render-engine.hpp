#pragma once

#include <lava/crater/VideoMode.hpp>

namespace lava {
    class IRenderTarget;
}

namespace lava {
    /**
     * An engine that manages everything that need to be drawn.
     */
    class RenderEngine {
    public:
        RenderEngine();
        ~RenderEngine();

        class Impl;
        Impl& impl() const { return *m_impl; }

        void draw();
        void update();

        void add(IRenderTarget& renderTarget);

    private:
        Impl* m_impl = nullptr;
    };
}
