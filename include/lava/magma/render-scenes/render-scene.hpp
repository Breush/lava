#pragma once

#include <lava/chamber/bucket-allocator.hpp>
#include <lava/magma/renderer-type.hpp>

#include <functional>
#include <memory>

namespace lava::magma {
    class RenderEngine;
    class ICamera;
    class ILight;
    class Material;
    class Mesh;
    class Texture;
}

namespace lava::magma {
    /**
     * A basic render scene for 3D elements.
     */
    class RenderScene {
    public:
        // @note Each frame will be renderer with a frame id being within [0 .. FRAME_IDS_COUNT],
        // this is independent from the swapchain and is incremented during each render scene update.
        // Consider using it when you update some buffer that might be used during current render,
        // which is the case with Shadows's ubo.
        // @fixme Move that to config file or something?
        static constexpr const uint32_t FRAME_IDS_COUNT = 3u;

    public:
        RenderScene(RenderEngine& engine);
        ~RenderScene();

        RenderEngine& engine() { return m_engine; }
        const RenderEngine& engine() const { return m_engine; }

        /// Choose the type of renderer.
        void rendererType(RendererType rendererType);

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
        void add(Material& material);
        void add(std::unique_ptr<Texture>&& texture);
        void add(Mesh& mesh);
        void add(std::unique_ptr<ILight>&& light);
        /// @}

        /**
         * @name Removers
         * Remove a previously added (or made) resource.
         */
        /// @{
        void remove(const Mesh& mesh);
        void remove(const Material& material);
        void remove(const Texture& texture);
        /// @}

        /**
         * @name Environment
         */
        /// @{
        /// The global texture (cube) to be used as environment map within shaders' epiphany.
        void environmentTexture(Texture* texture);
        /// @}

        /**
         * @name Allocators
         */
        /// @{
        chamber::BucketAllocator& materialAllocator() { return m_materialAllocator; }
        chamber::BucketAllocator& meshAllocator() { return m_meshAllocator; }
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

    private:
        RenderEngine& m_engine;

        // ----- Allocators
        chamber::BucketAllocator m_materialAllocator;
        chamber::BucketAllocator m_meshAllocator;
    };
}

#include <lava/magma/render-scenes/render-scene.inl>
