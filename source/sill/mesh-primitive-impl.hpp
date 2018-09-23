#pragma once

#include <lava/sill/mesh-primitive.hpp>

#include <lava/sill/game-engine.hpp>

namespace lava::sill {
    class MeshPrimitive::Impl {
    public:
        Impl(GameEngine& engine);
        ~Impl();

        magma::Mesh* magma() { return m_magma; }
        const magma::Mesh* magma() const { return m_magma; }

        // Geometry
        void verticesCount(const uint32_t count);
        void verticesPositions(VectorView<glm::vec3> positions);
        void verticesUvs(VectorView<glm::vec2> uvs);
        void verticesNormals(VectorView<glm::vec3> normals);
        void verticesTangents(VectorView<glm::vec4> tangents);
        void indices(VectorView<uint16_t> indices);

        // Material
        void material(Material& material);

    private:
        // References
        GameEngine::Impl& m_engine;
        Material* m_material = nullptr;

        // Resources
        magma::Mesh* m_magma = nullptr;
    };
}
