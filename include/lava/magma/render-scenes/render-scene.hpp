#pragma once

#include <lava/magma/render-scenes/i-render-scene.hpp>

#include <functional>
#include <memory>

namespace lava::magma {
    class RenderEngine;
    class ICamera;
    class IMesh;
    class ILight;
    class Material;
    class Texture;
}

namespace lava::magma {
    /**
     * A basic render scene.
     */
    class RenderScene final : public IRenderScene {
    public:
        RenderScene(RenderEngine& engine);
        ~RenderScene();

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
        void add(std::unique_ptr<Material>&& material);
        void add(std::unique_ptr<Texture>&& texture);
        void add(std::unique_ptr<IMesh>&& mesh);
        void add(std::unique_ptr<ILight>&& light);
        /// @}

        /**
         * @name Removers
         * Remove a previously added (or made) resource.
         */
        /// @{
        void remove(const IMesh& mesh);
        void remove(const Material& material);
        void remove(const Texture& texture);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}

#include <lava/magma/render-scenes/render-scene.inl>
