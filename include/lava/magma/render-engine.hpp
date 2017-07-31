#pragma once

#include <memory>

namespace lava::magma {
    class ICamera;
    class IMaterial;
    class IMesh;
    class IPointLight;
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

        /// Render the current state to all the targets.
        void draw();

        /// Update the internal state, mainly animations.
        void update();

        /**
         * @name Makers
         * Make a new resource and add it to the engine.

         * Arguments will be forwarded to the constructor.
         * Any resource that match an adder (see below) can be made.
         *
         * ```
         * auto& mesh = engine.make<Mesh>(); // Its lifetime is now managed by the engine.
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);
        /// Make a new resource using a custom maker.
        template <class T, class... Arguments>
        T& make(std::function<void(T&)> maker, Arguments&&... arguments);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<IMaterial>&& material);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(std::unique_ptr<IPointLight>&& pointLight);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-engine.inl>
