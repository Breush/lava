#pragma once

#include <memory>

namespace lava::magma {
    class ICamera;
    class IMaterial;
    class IMesh;
    class IRenderTarget;
}

namespace lava::magma {
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
         *      auto& mesh = engine.make<Mesh>();
         */
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);

        /**
         * Make a new resource using a custom maker.
         */
        template <class T, class... Arguments>
        T& make(std::function<void(T&)> maker, Arguments&&... arguments);

        /**
         * Add resource that has already been created.
         */
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<IMaterial>&& material);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(IRenderTarget& renderTarget);

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-engine.inl>
