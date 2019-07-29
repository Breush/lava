#pragma once

#include <lava/chamber/bucket-allocator.hpp>
#include <lava/core/extent.hpp>
#include <lava/core/macros/aft.hpp>
#include <lava/magma/renderer-type.hpp>

namespace lava::magma {
    class SceneAft;
    class RenderEngine;
    class Light;
    class Camera;
    class Material;
    class Texture;
    class Mesh;
}

namespace lava::magma {
    /**
     * A scene to render 3D meshes.
     */
    class Scene {
    public:
        Scene(RenderEngine& engine);
        ~Scene();

        $aft_class(Scene);

        /**
         * @name References
         */
        /// @{
        RenderEngine& engine() { return m_engine; }
        const RenderEngine& engine() const { return m_engine; }
        /// @}

        /**
         * @name Rendering
         */
        /// @{
        /**
         * Choose the type of renderer.
         *
         * Each new camera will be renderered using the specified renderer.
         */
        RendererType rendererType() const { return m_rendererType; }
        void rendererType(RendererType rendererType) { m_rendererType = rendererType; }
        /// @}

        /**
         * @name Makers
         *
         * Allocate a new resource and add it to the scene.
         *
         * Arguments will be forwarded to the constructor.
         * Any resource that match an makeResource (see below) can be made.
         *
         * ```
         * auto& mesh = engine.make<Mesh>(); // Its lifetime is now managed by the engine.
         * ```
         */
        /// @{
        /// Make a new resource directly.
        template <class T, class... Arguments>
        T& make(Arguments&&... arguments);

        Light& makeLight();
        Camera& makeCamera(Extent2d extent);
        Material& makeMaterial(const std::string& hrid);
        Texture& makeTexture();
        Mesh& makeMesh();
        /// @}

        /**
         * @name Removers
         *
         * Remove a previously added resource.
         */
        /// @{
        void remove(const Light& light);
        void remove(const Camera& camera);
        void remove(const Material& material);
        void remove(const Texture& texture);
        void remove(const Mesh& mesh);

        /**
         * Remove the resource without considering the backend (SceneAft).
         * These are not expected to be called outside the backend
         * internal routine.
         */
        void removeUnsafe(const Light& light);
        void removeUnsafe(const Camera& camera);
        void removeUnsafe(const Material& material);
        void removeUnsafe(const Texture& texture);
        void removeUnsafe(const Mesh& mesh);
        /// @}

        /**
         * @name Accessors
         */
        /// @{
        const std::vector<Light*>& lights() const { return m_lights; }
        const std::vector<Camera*>& cameras() const { return m_cameras; }
        const std::vector<Material*>& materials() const { return m_materials; }
        const std::vector<Texture*>& textures() const { return m_textures; }
        const std::vector<Mesh*>& meshes() const { return m_meshes; }
        /// @}

        /**
         * @name Environment
         */
        /// @{
        /// The global texture (cube) to be used as environment map within shaders' epiphany.
        void environmentTexture(const Texture* texture);
        /// @}

        /**
         * @name Fallbacks
         */
        /// @{
        /// Automatically created material with hrid "fallback".
        Material& fallbackMaterial() { return *m_fallbackMaterial; }
        /// @}

        /**
         * @name Allocators
         *
         * To be used whenever one wants to allocate a resource.
         * However, the make<Resource>() might be more convenient,
         * as it calls the add() function too.
         */
        /// @{
        chamber::BucketAllocator& lightAllocator() { return m_lightAllocator; }
        chamber::BucketAllocator& cameraAllocator() { return m_cameraAllocator; }
        chamber::BucketAllocator& materialAllocator() { return m_materialAllocator; }
        chamber::BucketAllocator& textureAllocator() { return m_textureAllocator; }
        chamber::BucketAllocator& meshAllocator() { return m_meshAllocator; }
        /// @}

    private:
        // ----- References
        RenderEngine& m_engine;

        // ----- Rendering
        RendererType m_rendererType = RendererType::Unknown;

        // ----- Fallbacks
        Material* m_fallbackMaterial = nullptr;

        // ----- Allocators
        chamber::BucketAllocator m_lightAllocator;
        chamber::BucketAllocator m_cameraAllocator;
        chamber::BucketAllocator m_materialAllocator;
        chamber::BucketAllocator m_textureAllocator;
        chamber::BucketAllocator m_meshAllocator;

        // ----- Resources
        // These raw pointers are pointing to bucket allocators' adresses.
        std::vector<Light*> m_lights;
        std::vector<Camera*> m_cameras;
        std::vector<Material*> m_materials;
        std::vector<Texture*> m_textures;
        std::vector<Mesh*> m_meshes;
    };
}

#include <lava/magma/scene.inl>
