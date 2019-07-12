#pragma once

// @fixme Check if everything is needed
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <lava/core/axis.hpp>
#include <lava/core/vector-view.hpp>
#include <lava/magma/aft-infos.hpp>
#include <lava/magma/bounding-sphere.hpp>
#include <lava/magma/vertex.hpp>

namespace lava::magma {
    class MeshAft;
    class Material;
    class RenderScene;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh {
    public:
        Mesh(RenderScene& scene);
        ~Mesh();

        /// Internal implementation
        MeshAft& aft() { return reinterpret_cast<MeshAft&>(m_aft); }
        const MeshAft& aft() const { return reinterpret_cast<const MeshAft&>(m_aft); }

        /**
         * @name Transform
         */
        /// @{
        /// The transform is `translation * rotation * scaling`.
        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);

        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation);
        void translate(const glm::vec3& delta) { translation(m_translation + delta); }

        const glm::quat& rotation() const { return m_rotation; }
        void rotation(const glm::quat& rotation);
        void rotate(const glm::vec3& axis, float delta);

        const glm::vec3& scaling() const { return m_scaling; }
        void scaling(const glm::vec3& scaling);
        void scaling(float commonScaling) { scaling(glm::vec3(commonScaling)); }
        void scale(const glm::vec3& delta) { scaling(m_scaling * delta); }
        void scale(float delta) { scaling(m_scaling * delta); }
        /// @}

        /**
         * @name Bounding sphere
         */
        /// @{
        /// Geometry-space bounding sphere, which might be slightly overestimated.
        const BoundingSphere& boundingSphereGeometry() const { return m_boundingSphereGeometry; }
        /// World-space bounding sphere, which might be slightly overestimated.
        const BoundingSphere& boundingSphere() const { return m_boundingSphere; }
        /// @}

        /**
         * @name Geometry
         */
        /// @{
        void verticesCount(const uint32_t count);
        void verticesPositions(VectorView<glm::vec3> positions);
        void verticesUvs(VectorView<glm::vec2> uvs);
        void verticesNormals(VectorView<glm::vec3> normals);
        void verticesTangents(VectorView<glm::vec4> tangents);
        void indices(VectorView<uint16_t> indices, bool flipTriangles = false);
        void indices(VectorView<uint8_t> indices, bool flipTriangles = false);

        const std::vector<UnlitVertex>& unlitVertices() const { return m_unlitVertices; };
        const std::vector<Vertex>& vertices() const { return m_vertices; };
        const std::vector<uint16_t>& indices() const { return m_indices; };

        /// From current vertices positions, compute flat normals.
        void computeFlatNormals();

        /// From current vertices positions and normals, compute tangents.
        void computeTangents();
        /// @}

        /**
         * @name Material
         */
        /// @{
        /// The mesh's material. Can be nullptr.
        Material* material() { return m_material; }
        const Material* material() const { return m_material; }
        void material(Material& material);

        /**
         * Whether the mesh should be considered translucent.
         * If so, it will go to a specific pipeline to be
         * rendered with alpha blending.
         */
        bool translucent() const { return m_translucent; }
        void translucent(bool translucent) { m_translucent = translucent; }

        /// Whether the mesh can cast shadows.
        bool shadowsCastable() const { return m_shadowsCastable; }
        void shadowsCastable(bool shadowsCastable) { m_shadowsCastable = shadowsCastable; }

        /// Render the mesh as wireframe only.
        bool wireframed() const { return m_wireframed; }
        void wireframed(bool wireframed) { m_wireframed = wireframed; }

        /**
         * When a mesh is depthless, it will be renderered
         * behind all other objects and centered at the camera
         * point of view. The rotation of the camera is the
         * only thing that matters then.
         * This is intended to be used with skyboxes.
         */
        bool depthless() const { return m_depthless; }
        void depthless(bool depthless) { m_depthless = depthless; }

        /// Whether the mesh should be rendered if the render target is a VR one.
        bool vrRenderable() const { return m_vrRenderable; }
        void vrRenderable(bool vrRenderable) { m_vrRenderable = vrRenderable; }
        /// @}

        /**
         * @name Debug
         */
        /// @{
        /// Add a wireframed mesh to show the bounding sphere.
        bool debugBoundingSphere() const { return m_debugBoundingSphere; }
        void debugBoundingSphere(bool debugBoundingSphere);
        /// @}

    private:
        void updateTransform();
        void updateBoundingSphere();

    private:
        uint8_t m_aft[MAGMA_SIZEOF_MeshAft];

        // ----- References
        RenderScene& m_scene;

        // ----- Transform
        glm::mat4 m_transform = glm::mat4(1.f);
        glm::vec3 m_translation = glm::vec3(0.f);
        glm::quat m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 m_scaling = glm::vec3(1.f);

        // ----- Bounding sphere
        BoundingSphere m_boundingSphereGeometry;
        BoundingSphere m_boundingSphere;
        // Geometry-space bounding box dimenstion which is centered at m_boundingSphereGeometry.center.
        glm::vec3 m_boundingBoxExtentGeometry;

        // ----- Geometry

        std::vector<Vertex> m_temporaryVertices; // Only used for tangents generation.
        std::vector<UnlitVertex> m_unlitVertices;
        std::vector<Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        // ----- Material
        Material* m_material = nullptr;
        bool m_translucent = false;
        bool m_shadowsCastable = true;
        bool m_wireframed = false;
        bool m_depthless = false;
        bool m_vrRenderable = true;

        // ----- Debug
        bool m_debugBoundingSphere = false;
        Mesh* m_debugBoundingSphereMesh = nullptr;
    };
}
