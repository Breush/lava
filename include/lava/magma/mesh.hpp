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

        /**
         * @name Transforms
         */
        /// @{
        /// The transform combines scaling, rotation and translation.
        const glm::mat4& transform() const;
        void transform(const glm::mat4& transform);

        glm::vec3 translation() const;
        void translation(const glm::vec3& translation);
        void translate(const glm::vec3& delta);

        // @todo Return rotation, as a glm::quat
        void rotate(const glm::vec3& axis, float angleDelta);

        void scale(float factor);
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
