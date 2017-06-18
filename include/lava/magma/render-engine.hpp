#pragma once

#include <lava/crater/VideoMode.hpp>
#include <memory>

namespace lava {
    class IRenderTarget;
    class MrrMaterial;
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
        MrrMaterial& add(std::unique_ptr<MrrMaterial>&& material);

    private:
        Impl* m_impl = nullptr;
    };
}
