#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <functional>
#include <memory>

namespace lava::magma {
    class RenderEngine;
    class ICamera;
    class IMaterial;
    class IMesh;
    class ILight;
}

namespace lava::magma {
    /**
     * A basic render scene.
     */
    class RenderScene final : public IRenderScene {
    public:
        RenderScene(RenderEngine& engine);
        RenderScene(RenderEngine& engine, Extent2d extent);
        ~RenderScene();

        // IRenderScene
        Extent2d extent() const override final;
        void extent(Extent2d extent) override final;

        IRenderScene::Impl& interfaceImpl() override final;

        /**
         * @name Makers
         * Make a new resource and add it to the scene.

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
         * Its ownership goes to the scene.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(std::unique_ptr<ICamera>&& camera);
        void add(std::unique_ptr<IMaterial>&& material);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(std::unique_ptr<ILight>&& light);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-scenes/render-scene.inl>
