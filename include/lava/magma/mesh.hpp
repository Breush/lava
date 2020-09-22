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
     *
     * The ubos vector contains info about all instances.
     */
    class Mesh {
    public:
        Mesh(Scene& scene, uint32_t instancesCount);
        ~Mesh();

        $aft_class(Mesh);

        Scene& scene() { return m_scene; }
        const Scene& scene() const { return m_scene; }

        /// Whether the mesh should be rendered.
        bool enabled() const { return m_enabled && (m_ubos.size() > 0u); }
        void enabled(bool enabled) { m_enabled = enabled; }

        /**
         * @name Transform
         */
        /// @{
        // @todo :Terminology Should this still be called transform? Or is that term reserved for uniform scaling lava::Transform?
        /// The transform is `translation * rotation * scaling`.
        const glm::mat4& transform(uint32_t instanceIndex = 0u) const { return m_transformsInfos.at(instanceIndex).transform; }
        void transform(const glm::mat4& transform, uint32_t instanceIndex = 0u);

        const glm::vec3& translation(uint32_t instanceIndex = 0u) const { return m_transformsInfos.at(instanceIndex).translation; }
        void translation(const glm::vec3& translation, uint32_t instanceIndex = 0u);
        void translate(const glm::vec3& delta, uint32_t instanceIndex = 0u) { translation(m_transformsInfos.at(instanceIndex).translation + delta, instanceIndex); }

        const glm::quat& rotation(uint32_t instanceIndex = 0u) const { return m_transformsInfos.at(instanceIndex).rotation; }
        void rotation(const glm::quat& rotation, uint32_t instanceIndex = 0u);
        void rotate(const glm::vec3& axis, float angle, uint32_t instanceIndex = 0u)
        {
            rotation(glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), angle, axis) * m_transformsInfos.at(instanceIndex).rotation, instanceIndex);
        }

        const glm::vec3& scaling(uint32_t instanceIndex = 0u) const { return m_transformsInfos.at(instanceIndex).scaling; }
        void scaling(const glm::vec3& scaling, uint32_t instanceIndex = 0u);
        void scaling(float commonScaling, uint32_t instanceIndex = 0u) { scaling(glm::vec3(commonScaling), instanceIndex); }
        void scale(const glm::vec3& delta, uint32_t instanceIndex = 0u) { scaling(m_transformsInfos.at(instanceIndex).scaling * delta, instanceIndex); }
        void scale(float delta, uint32_t instanceIndex = 0u) { scaling(m_transformsInfos.at(instanceIndex).scaling * delta, instanceIndex); }
        /// @}

        /**
         * @name Bounding sphere
         */
        /// @{
        /// World-space bounding sphere, which might be slightly overestimated.
        /// This englobes all instances.
        const BoundingSphere& boundingSphere() {
            if (m_boundingSphereDirty) {
                updateBoundingSphere();
            }
            return m_boundingSphere;
        }
        /// @}

        /**
         * @name Instancing
         */
        /// @{
        /// How many instances to be rendered. Always at least 1.
        uint32_t instancesCount() const { return m_ubos.size(); }
        /// The mesh will be rendered once more. Set individual matrices thanks to instanceIndex.
        uint32_t addInstance();
        /// Remove the last instance. Reduces the instance count by one.
        void removeInstance();
        /// Warn in advance how many instances the mesh will have.
        void reserveInstancesCount(uint32_t instancesCount);
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
        RenderCategory renderCategory() const { return m_renderCategory; }
        void renderCategory(RenderCategory renderCategory) { m_renderCategory = renderCategory; }

        /// Whether the mesh should be rendered if the render target is a VR one.
        bool vrRenderable() const { return m_vrRenderable; }
        void vrRenderable(bool vrRenderable) { m_vrRenderable = vrRenderable; }
        /// @}

        /**
         * @name Shader data
         */
        /// @{
        const MeshUbo& ubo() const { return m_ubos.at(0); }
        const std::vector<MeshUbo>& ubos() const { return m_ubos; }
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
        void updateUbo(uint32_t instanceIndex);
        void updateTransform(uint32_t instanceIndex);
        void updateBoundingSphere();

    private:
        struct TransformInfo {
            glm::mat4 transform = glm::mat4(1.f);
            // Decompose values of above transform.
            glm::vec3 translation = glm::vec3(0.f);
            glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
            glm::vec3 scaling = glm::vec3(1.f);
        };

    private:
        // ----- References
        Scene& m_scene;
        bool m_enabled = true;

        // ----- Transform
        std::vector<TransformInfo> m_transformsInfos;

        // ----- Bounding sphere
        BoundingSphere m_boundingSphereGeometry;
        BoundingSphere m_boundingSphere;
        // Geometry-space bounding box dimenstion which is centered at m_boundingSphereGeometry.center.
        glm::vec3 m_boundingBoxExtentGeometry;
        bool m_boundingSphereDirty = true;

        // ----- Geometry
        std::vector<Vertex> m_temporaryVertices; // Only used for tangents generation.
        std::vector<UnlitVertex> m_unlitVertices;
        std::vector<Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        // ----- Material
        MaterialPtr m_material = nullptr;
        RenderCategory m_renderCategory = RenderCategory::Opaque;
        bool m_translucent = false;
        bool m_shadowsCastable = true;
        bool m_vrRenderable = true;

        // ----- Shader data
        std::vector<MeshUbo> m_ubos;

        // ----- Debug
        bool m_debugBoundingSphere = false;
        Mesh* m_debugBoundingSphereMesh = nullptr;
    };
}
