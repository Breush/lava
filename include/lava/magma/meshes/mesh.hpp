#pragma once

#include <lava/magma/meshes/i-mesh.hpp>

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
    class Mesh final : public IMesh {
    public:
        Mesh(RenderScene& scene);
        ~Mesh();

        // IMesh
        IMesh::Impl& interfaceImpl() override final;

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

        glm::vec3 scaling() const;
        void scaling(const glm::vec3& scaling);
        void scale(const glm::vec3& factors);
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
        void indices(VectorView<uint16_t> indices);
        /// @}

        /**
         * @name Material
         */
        /// @{
        Material& material();
        void material(Material& material);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
