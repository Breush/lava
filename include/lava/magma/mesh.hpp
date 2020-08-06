#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <lava/core/bounding-sphere.hpp>
#include <lava/core/macros/aft.hpp>
#include <lava/core/render-category.hpp>
#include <lava/core/vector-view.hpp>
#include <lava/magma/ubos.hpp>
#include <lava/magma/vertex.hpp>

namespace lava::magma {
    class MeshAft;
    class Material;
    class Scene;

    using MaterialPtr = std::shared_ptr<Material>;
}

namespace lava::magma {
    /**
     * A mesh, holding geometry and transform.
     */
    class Mesh {
    public:
        Mesh(Scene& scene);
        ~Mesh();

        $aft_class(Mesh);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /// Whether the mesh should be rendered.
        bool enabled() const { return m_enabled; }
        void enabled(bool enabled) { m_enabled = enabled; }

        /**
         * @name Transform
         */
        /// @{
        // @todo :Terminology Should this still be called transform? Or is that term reserved for uniform scaling lava::Transform?
        /// The transform is `translation * rotation * scaling`.
        const glm::mat4& transform() const { return m_transform; }
        void transform(const glm::mat4& transform);

        const glm::vec3& translation() const { return m_translation; }
        void translation(const glm::vec3& translation);
        void translate(const glm::vec3& delta) { translation(m_translation + delta); }

        const glm::quat& rotation() const { return m_rotation; }
        void rotation(const glm::quat& rotation);
        void rotate(const glm::vec3& axis, float angle)
        {
            rotation(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), angle, axis) * m_rotation);
        }

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
        uint32_t verticesCount() const { return m_vertices.size(); }
        void verticesCount(const uint32_t count);
        void verticesPositions(const VectorView<glm::vec3>& positions);
        void verticesUvs(const VectorView<glm::vec2>& uvs);
        void verticesNormals(const VectorView<glm::vec3>& normals);
        void verticesTangents(const VectorView<glm::vec4>& tangents);
        void indices(const VectorView<uint32_t>& indices, bool flipTriangles = false);
        void indices(const VectorView<uint16_t>& indices, bool flipTriangles = false);
        void indices(const VectorView<uint8_t>& indices, bool flipTriangles = false);

        const std::vector<UnlitVertex>& unlitVertices() const { return m_unlitVertices; };
        const std::vector<Vertex>& vertices() const { return m_vertices; };
        const std::vector<uint16_t>& indices() const { return m_indices; };
        std::vector<UnlitVertex>& unlitVertices() { return m_unlitVertices; };
        std::vector<Vertex>& vertices() { return m_vertices; };
        std::vector<uint16_t>& indices() { return m_indices; };

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
        MaterialPtr material() { return m_material; }
        void material(MaterialPtr material);

        /// Whether the mesh can cast shadows.
        bool shadowsCastable() const { return m_shadowsCastable; }
        void shadowsCastable(bool shadowsCastable) { m_shadowsCastable = shadowsCastable; }

        /// Decides how to render the mesh.
        RenderCategory category() { return m_category; }
        void category(RenderCategory category) { m_category = category; }

        /// Whether the mesh should be rendered if the render target is a VR one.
        bool vrRenderable() const { return m_vrRenderable; }
        void vrRenderable(bool vrRenderable) { m_vrRenderable = vrRenderable; }
        /// @}

        /**
         * @name Shader data
         */
        /// @{
        const MeshUbo& ubo() const { return m_ubo; }
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
        void updateUbo();
        void updateTransform();
        void updateBoundingSphere();

    private:
        // ----- References
        Scene& m_scene;
        bool m_enabled = true;

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
        MaterialPtr m_material = nullptr;
        RenderCategory m_category = RenderCategory::Opaque;
        bool m_translucent = false;
        bool m_shadowsCastable = true;
        bool m_vrRenderable = true;

        // ----- Shader data
        MeshUbo m_ubo;

        // ----- Debug
        bool m_debugBoundingSphere = false;
        Mesh* m_debugBoundingSphereMesh = nullptr;
    };
}
