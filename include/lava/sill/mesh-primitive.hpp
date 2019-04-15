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
         * @name Materials
         */
        /// @{
        void material(Material& material);
        void translucent(bool translucent);
        /// @}

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
