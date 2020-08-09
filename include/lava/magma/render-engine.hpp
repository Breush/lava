#pragma once

#include <functional>
#include <glm/mat4x4.hpp>
#include <lava/chamber/bucket-allocator.hpp>
#include <lava/core/filesystem.hpp>
#include <lava/core/viewport.hpp>
#include <lava/magma/material-info.hpp>
#include <lava/magma/vr-engine.hpp>
#include <memory>

namespace lava::magma {
    class Camera;
    class IRenderTarget;
    class RenderImage;
    class Scene;
    class Texture;

    using TexturePtr = std::shared_ptr<Texture>;
}

namespace lava::magma {
    /**
     * An engine that manages everything that need to be drawn.
     */
    class RenderEngine {
    public:
        RenderEngine();
        ~RenderEngine();

        /// Update animations and internal state.
        void update();

        /// Render the current state to all the targets.
        void draw();

        /// Receive all infos about a parsed registered material.
        const MaterialInfo& materialInfo(const std::string& hrid) const;
        const MaterialInfo* materialInfoIfExists(const std::string& hrid) const;

        /**
         * Register a material (.shmag) to the engine.
         *
         * All materials have to be registered before adding anything,
         * especially render scenes or render targets.
         */
        uint32_t registerMaterialFromFile(const std::string& hrid, const fs::Path& shaderPath);

        /**
         * Add a view of render scene's camera to a render-target.
         * One can show multiple scenes to the same target
         * or the same scene to multiple targets.
         * Everything is possible.
         *
         * @return A unique identifier to the view generated.
         */
        uint32_t addView(Camera& camera, IRenderTarget& renderTarget, const Viewport& viewport);
        uint32_t addView(RenderImage renderImage, IRenderTarget& renderTarget, const Viewport& viewport);
        void removeView(uint32_t viewId);

        /**
         * @name Makers
         * Make a new resource and add it to the engine.
         *
         * Arguments will be forwarded to the constructor.
         * Any resource that match an adder (see below) can be made.
         *
         * ```
         * auto& scene = engine.make<Scene>(); // Its lifetime is now managed by the engine.
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);

        Scene& makeScene();
        TexturePtr makeTexture(const std::string& imagePath = "");

        /// Find an existing texture which has the same pixels.
        /// @note This done through an hash computation and can give a wrong texture sometimes.
        TexturePtr findTexture(const uint8_t* pixels, uint32_t width, uint32_t height, uint8_t channels);
        /// @}

        /**
         * @name Adders
         * Add a resource that has already been created.
         *
         * Its ownership goes to the engine.
         * For convenience, you usually want to use makers (see above).
         */
        /// @{
        void add(Scene& scene);
        void add(std::unique_ptr<IRenderTarget>&& renderTarget);
        /// @}

        /**
         * @name VR
         */
        /// @{
        VrEngine& vr() { return m_vrEngine; }
        const VrEngine& vr() const { return m_vrEngine; }
        /// @}

        /// Enable extra logging for next draw.
        void logTrackingOnce();

        /**
         * @name Allocators
         *
         * To be used whenever one wants to allocate a resource.
         * However, the make<Resource>() might be more convenient,
         * as it calls the add() function too.
         */
        /// @{
        chamber::BucketAllocator& sceneAllocator() { return m_sceneAllocator; }
        chamber::BucketAllocator& textureAllocator() { return m_textureAllocator; }
        /// @}

    protected:
        // Custom deleter for ResourcePtr. Internal usage.
        void forget(Texture& texture);

    public:
        class Impl;
        const Impl& impl() const { return *m_impl; }
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;

        // VR
        VrEngine m_vrEngine;

        // Allocators
        chamber::BucketAllocator m_sceneAllocator;
        chamber::BucketAllocator m_textureAllocator;

        // ----- Resources
        // These raw pointers are pointing to bucket allocators' adresses.
        std::unordered_map<Texture*, std::weak_ptr<Texture>> m_textures;
    };
}

#include <lava/magma/render-engine.inl>
