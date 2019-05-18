#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <lava/core/vector-view.hpp>

namespace lava::sill {
    class Material;
    class GameEngine;
}

namespace lava::sill {
    /**
     * Holds the geometry and material information.
     */
    class MeshPrimitive {
    public:
        MeshPrimitive(GameEngine& engine);
        MeshPrimitive(MeshPrimitive&& meshPrimitive);
        ~MeshPrimitive();

        void computeFlatNormals();
        void computeTangents();

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
         * @name Materials
         */
        /// @{
        Material& material();
        void material(Material& material);
        void translucent(bool translucent);
        /// @}

        /**
         * @name Debug
         */
        /// @{
        bool wireframed() const;
        void wireframed(bool wireframed);

        bool boundingSphereVisible() const;
        void boundingSphereVisible(bool boundingSphereVisible);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
