#pragma once

#include <glm/glm.hpp>
#include <lava/core/axis.hpp>
#include <lava/core/vector-view.hpp>
#include <string>
#include <vector>

namespace lava::magma {
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

        // @fixme Documentation
        bool canCastShadows() const;
        void canCastShadows(bool canCastShadows);

        /// Whether the mesh should be rendered if the render target is a VR one.
        bool vrRenderable() const;
        void vrRenderable(bool vrRenderable);

        /// From current vertices positions, compute flat normals.
        void computeFlatNormals();

        /// From current vertices positions and normals, compute tangents.
        void computeTangents();

        /**
         * @name Transforms
         */
        /// @{
        /// The transform is `translation * rotation * scaling`.
        const glm::mat4& transform() const;
        void transform(const glm::mat4& transform);

        const glm::vec3& translation() const;
        void translation(const glm::vec3& translation);
        void translate(const glm::vec3& delta);

        const glm::quat& rotation() const;
        void rotation(const glm::quat& rotation);
        void rotate(const glm::vec3& axis, float angleDelta);

        const glm::vec3& scaling() const;
        void scaling(const glm::vec3& scaling);
        void scaling(float scaling);
        void scale(const glm::vec3& scaling);
        void scale(float scaling);
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
        /// @}

        /**
         * @name Material
         */
        /// @{
        Material& material();
        void material(Material& material);

        /**
         * Whether the mesh should be considered translucent.
         * If so, it will go to a specific pipeline to be
         * rendered with alpha blending.
         */
        bool translucent() const;
        void translucent(bool translucent);

        /**
         * When a mesh is depthless, it will be renderered
         * behind all other objects and centered at the camera
         * point of view. The rotation of the camera is the
         * only thing that matters then.
         * This is intended to be used with skyboxes.
         */
        bool depthless() const;
        void depthless(bool depthless);
        /// @}

        /**
         * @name Debug
         */
        /// @{
        bool wireframed() const;
        void wireframed(bool wireframed);

        /// Add a wireframed mesh to show the bounding sphere.
        bool boundingSphereVisible() const;
        void boundingSphereVisible(bool boundingSphereVisible);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }
        const Impl& impl() const { return *m_impl; }

    private:
        RenderScene& m_scene;
        Impl* m_impl = nullptr;
    };
}
