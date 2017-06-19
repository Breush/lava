#pragma once

#include <lava/crater/VideoMode.hpp>
#include <memory>

namespace lava {
    class IRenderTarget;
    class IMaterial;
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

        /**
         * Make a new resource and add it to the engine.
         *
         * @example
         *      auto& material = engine.make<MrrMaterial>();
         */
        template <class T>
        T& make();

        /**
         * Add resource that has already been created.
         */
        void add(IRenderTarget& renderTarget);
        IMaterial& add(std::unique_ptr<IMaterial>&& material);

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-engine.inl>
